/*
 * Copyright (c) 2014-2015, dennis wang
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dennis wang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "framework.h"
#include "loop.h"
#include "loop_balancer.h"
#include "channel_ref.h"
#include "framework_raiser.h"
#include "framework_worker.h"
#include "rpc_api.h"
#include "list.h"
#include "misc.h"

struct _framework_t {
    dlist_t*             loops;    /* 网络事件循环 */
    framework_config_t*  c;        /* 配置 */
    framework_raiser_t*  raiser;   /* 监听器/连接器 */
    framework_worker_t** workers;  /* 工作线程 */
    loop_balancer_t*     balancer; /* 负载均衡器 */
    int                  start;    /* 启动标志 */
};

/**
 * 关闭并销毁所有线程
 * @param f framework_t实例
 */
void _cleanup_all_threads(framework_t* f);

/**
 * 启动工作线程
 * @param f framework_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _start_worker_threads(framework_t* f);

/**
 * 启动监听器/连接器发起线程
 * @param f framework_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _start_raiser_thread(framework_t* f);

framework_t* framework_create() {
    framework_t* f = create(framework_t);
    verify(f);
    memset(f, 0, sizeof(framework_t));
    /* 建立配置器 */
    f->c = framework_config_create();
    /* 建立事件循环链表 */
    f->loops = dlist_create();
    verify(f->c);
    return f;
}

void framework_destroy(framework_t* f) {
    dlist_node_t* node = 0;
    dlist_node_t* temp = 0;
    loop_t*       loop = 0;
    verify(f);
    if (f->start) { /* 未关闭 */
        framework_stop(f);
    }
    /* 销毁所有线程 */
    _cleanup_all_threads(f);
    /* 销毁负载均衡器 */
    if (f->balancer) {
        loop_balancer_destroy(f->balancer);
    }
    /* 销毁配置器 */
    if (f->c) {
        framework_config_destroy(f->c);
    }
    /* 销毁loop */
    dlist_for_each_safe(f->loops, node, temp) {
        loop = (loop_t*)dlist_node_get_data(node);
        loop_destroy(loop);
    }
    dlist_destroy(f->loops);
    destroy(f->workers);
    destroy(f);
}

int framework_start(framework_t* f) {
    int           i            = 0;
    int           error        = 0;
    int           worker_count = 0;
    loop_t*       loop         = 0;
    verify(f);
    verify(f->c);
    worker_count = framework_config_get_worker_thread_count(f->c);
    /* 建立负载均衡器 */
    f->balancer = loop_balancer_create();
    verify(f->balancer);
    /* 建立工作线程所需的所有事件循环 */
    for (; i < worker_count + 1; i++) {
        loop = loop_create();
        dlist_add_tail_node(f->loops, loop);
        /* 关联到负载均衡器 */
        loop_balancer_attach(f->balancer, loop);
    }
    /* 设置启动标志，无论是否启动线程 */
    f->start = 1;
    /* 启动监听器/连接器 */
    error = _start_raiser_thread(f);
    if (error_ok != error) {
        goto error_return;
    }
    /* 启动工作线程 */
    error = _start_worker_threads(f);
    if (error_ok != error) {
        goto error_return;
    }
    return error_ok;
error_return:
    /* 销毁监听器 */
    _cleanup_all_threads(f);
    f->start = 0;
    return error;
}

int framework_start_wait(framework_t* f) {
    int error = framework_start(f);
    if (error_ok == error) {
        framework_wait_for_stop(f);
    }
    return error;
}

int framework_start_wait_destroy(framework_t* f) {
    int error = framework_start_wait(f);
    framework_destroy(f);
    return error;
}

void framework_wait_for_stop(framework_t* f) {
    int i = 0;
    verify(f);
    if (!f->start) {
        return;
    }
    if (f->raiser) {
        framework_raiser_wait_for_stop(f->raiser);
    }
    /* 等待工作线程结束 */
    if (f->workers) {
        for (; i < framework_config_get_worker_thread_count(f->c); i++) {
            if (f->workers[i]) {
                framework_worker_wait_for_stop(f->workers[i]);
            }
        }
    }
    f->start = 0;
}

void framework_wait_for_stop_destroy(framework_t* f) {
    verify(f);
    framework_wait_for_stop(f);
    framework_destroy(f);
}

int framework_stop(framework_t* f) {
    int i = 0;
    verify(f);
    /* 先关闭监听器/连接器 */
    if (f->raiser) {
        framework_raiser_stop(f->raiser);
    }
    /* 关闭所有工作线程 */
    if (f->workers) {
        for (; i < framework_config_get_worker_thread_count(f->c); i++) {
            if (f->workers[i]) {
                framework_worker_stop(f->workers[i]);
            }
        }
    }
    return error_ok;
}

framework_config_t* framework_get_config(framework_t* f) {
    verify(f);
    return f->c;
}

loop_balancer_t* framework_get_balancer(framework_t* f) {
    verify(f);
    return f->balancer;
}

void _cleanup_all_threads(framework_t* f) {
    int i = 0;
    if (f->raiser) {
        framework_raiser_stop(f->raiser);
        framework_raiser_wait_for_stop(f->raiser);
        framework_raiser_destroy(f->raiser);
        f->raiser = 0;
    }
    /* 销毁工作线程 */
    if (f->workers) {
        for (; i < framework_config_get_worker_thread_count(f->c); i++) {
            if (f->workers[i]) {
                framework_worker_stop(f->workers[i]);
                framework_worker_wait_for_stop(f->workers[i]);
                framework_worker_destroy(f->workers[i]);
                f->workers[i] = 0;
            }
        }
    }
}

int _start_worker_threads(framework_t* f) {
    int           i     = 0;
    dlist_node_t* node  = 0;
    loop_t*       loop  = 0;
    int           error = error_ok;
    int worker_count = framework_config_get_worker_thread_count(f->c);
    f->workers = create_type(framework_worker_t*, worker_count * sizeof(framework_worker_t*));
    verify(f->workers);
    memset(f->workers, 0, worker_count * sizeof(framework_worker_t*));
    /* 建立工作线程 */
    dlist_for_each(f->loops, node) {
        loop = (loop_t*)dlist_node_get_data(node);
        if (i != 0) {
            f->workers[i - 1] = framework_worker_create(f, loop);
            verify(f->workers[i - 1]);
        }
        i++;
    }
    /* 启动工作线程 */
    for (i = 0; i < worker_count; i++) {
        error = framework_worker_start(f->workers[i]);
        if (error_ok != error) {
            return error;
        }
    }
    return error;
}

int _start_raiser_thread(framework_t* f) {
    dlist_node_t* node = dlist_get_front(f->loops);
    loop_t*       loop = (loop_t*)dlist_node_get_data(node);; 
    f->raiser = framework_raiser_create(f, loop);
    verify(f->raiser);
    return framework_raiser_start(f->raiser);
}
