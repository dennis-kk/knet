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
#include "framework_acceptor.h"
#include "framework_worker.h"
#include "misc.h"

struct _framework_config_t {
    char ip[32];              /* IP */
    int  port;                /* 监听端口 */
    int  worker_thread_count; /* 工作线程数量 */
    int  backlog;             /* listen() backlog */
    int  max_send_list;       /* 发送链表最大长度 */
    int  max_recv_buffer;     /* 接收缓冲区最大长度 */
    int  max_idle_timeout;    /* 心跳（秒） */
};

struct _framework_t {
    framework_config_t*   c;        /* 配置 */
    framework_acceptor_t* acceptor; /* 外部管道监听器 */
    framework_worker_t**  workers;  /* 工作线程 */
    loop_balancer_t*      balancer; /* 负载均衡器 */
    channel_ref_cb_t      cb;       /* 用户回调 */
    int                   start;    /* 启动标志 */
};

framework_t* framework_create() {
    framework_t* f = create(framework_t);
    verify(f);
    memset(f, 0, sizeof(framework_t));
    /* 建立配置器 */
    f->c = framework_config_create();
    verify(f->c);
    return f;
}

void framework_destroy(framework_t* f) {
    verify(f);
    if (f->start) { /* 未关闭 */
        framework_stop(f);
    }
    /* 销毁配置器 */
    if (f->c) {
        framework_config_destroy(f->c);
    }
    destroy(f->workers);
    destroy(f);
}

int framework_start(framework_t* f, channel_ref_cb_t cb) {
    int i            = 0;
    int error        = 0;
    int worker_count = 0;
    verify(f);
    verify(f->c);
    worker_count = framework_config_get_worker_thread_count(f->c);
    f->cb       = cb;
    f->balancer = loop_balancer_create(); /* 建立负载June和更年期 */
    verify(f->balancer);
    loop_balancer_set_data(f->balancer, f);
    f->workers = create_type(framework_worker_t*, worker_count * sizeof(framework_worker_t*));
    verify(f->workers);
    memset(f->workers, 0, worker_count * sizeof(framework_worker_t*));
    /* 启动所有工作线程 */
    for (; i < worker_count; i++) {
        f->workers[i] = framework_worker_create(f);
        verify(f->workers[i]);
        error = framework_worker_start(f->workers[i]);
        if (error_ok != error) {
            goto error_return;
        }
    }
    /* 启动监听器 */
    f->acceptor = framework_acceptor_create(f);
    verify(f->acceptor);
    error = framework_acceptor_start(f->acceptor);
    if (error_ok != error) {
        goto error_return;
    }
    f->start = 1;
    return error_ok;
error_return:
    framework_destroy(f);
    return error;
}

void framework_wait_for_stop(framework_t* f) {
    int i = 0;
    verify(f);
    framework_acceptor_wait_for_stop(f->acceptor);
    for (; i < framework_config_get_worker_thread_count(f->c); i++) {
        if (f->workers[i]) {
            framework_worker_wait_for_stop(f->workers[i]);
        }
    }
    /* 销毁负载均衡器 */
    if (f->balancer) {
        loop_balancer_destroy(f->balancer);
    }
    f->start = 0;
}

int framework_stop(framework_t* f) {
    int i = 0;
    verify(f);
    /* 先关闭监听器 */
    if (f->acceptor) {
        framework_acceptor_stop(f->acceptor);
    }
    /* 关闭所有工作线程 */
    for (; i < framework_config_get_worker_thread_count(f->c); i++) {
        framework_worker_stop(f->workers[i]);
    }
    return error_ok;
}

framework_config_t* framework_get_config(framework_t* f) {
    verify(f);
    return f->c;
}

void framework_config_set_address(framework_config_t* c, const char* ip, int port) {
    verify(c);
    verify(port);
    if (ip) {
        strcpy(c->ip, ip);
    } else {
        strcpy(c->ip, "0.0.0.0");
    }
    c->port = port;
}

const char* framework_config_get_ip(framework_config_t* c) {
    verify(c);
    return c->ip;
}

int framework_config_get_port(framework_config_t* c) {
    verify(c);
    return c->port;
}

void framework_config_set_backlog(framework_config_t* c, int backlog) {
    verify(c);
    c->backlog = backlog;
}

int framework_config_get_backlog(framework_config_t* c) {
    verify(c);
    return c->backlog;
}

void framework_config_set_worker_thread_count(framework_config_t* c, int worker_thread_count) {
    verify(c);
    verify(worker_thread_count);
    c->worker_thread_count = worker_thread_count;
}

void framework_config_set_max_send_list(framework_config_t* c, int max_send_list) {
    verify(c);
    c->max_send_list = max_send_list;
}

int framework_config_get_max_send_list(framework_config_t* c) {
    verify(c);
    return c->max_send_list;
}

void framework_config_set_max_recv_buffer(framework_config_t* c, int max_recv_buffer) {
    verify(c);
    c->max_recv_buffer = max_recv_buffer;
}

int framework_config_get_max_recv_buffer(framework_config_t* c) {
    verify(c);
    return c->max_recv_buffer;
}

void framework_config_set_max_idle_timeout(framework_config_t* c, int max_idle_timeout) {
    verify(c);
    c->max_idle_timeout = max_idle_timeout;
}

int framework_config_get_worker_thread_count(framework_config_t* c) {
    verify(c);
    return c->worker_thread_count;
}

framework_config_t* framework_config_create() {
    framework_config_t* c = create(framework_config_t);
    verify(c);
    memset(c, 0, sizeof(framework_config_t));
    c->backlog             = 100;       /* 默认 - 监听等待队列 */
    c->worker_thread_count = 1;         /* 默认 - 只有一个工作线程 */
    c->max_send_list       = 64;        /* 默认 - 发送链表最大元素数 */
    c->max_recv_buffer     = 1024 * 16; /* 默认 - 接受缓冲区16k */
    return c;
}

void framework_config_destroy(framework_config_t* c) {
    verify(c);
    destroy(c);
}

loop_balancer_t* framework_get_balancer(framework_t* f) {
    verify(f);
    return f->balancer;
}

channel_ref_cb_t framework_get_cb(framework_t* f) {
    verify(f);
    return f->cb;
}
