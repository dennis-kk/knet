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

#include "loop_balancer.h"
#include "list.h"
#include "misc.h"
#include "loop.h"


typedef struct _loop_info_t {
    loop_t*  loop;    /* loop_t实例 */
    uint64_t choose;  /* 被选取次数，保留 */
} loop_info_t;

struct _loop_balancer_t {
    dlist_t* loop_info_list; /* loop_t实例链表 */
    lock_t*  lock;           /* 锁 - 链表访问, loop_t实例的添加删除 */
    void*    data;           /* 用户数据 */
};

loop_balancer_t* loop_balancer_create() {
    loop_balancer_t* balancer = create(loop_balancer_t);
    verify(balancer);
    memset(balancer, 0, sizeof(loop_balancer_t));
    balancer->loop_info_list = dlist_create();
    verify(balancer->loop_info_list);
    balancer->lock = lock_create();
    verify(balancer->lock);
    return balancer;
}

void loop_balancer_destroy(loop_balancer_t* balancer) {
    dlist_node_t* node = 0;
    dlist_node_t* temp = 0;
    verify(balancer);
    lock_destroy(balancer->lock);
    dlist_for_each_safe(balancer->loop_info_list, node, temp) {
        destroy(dlist_node_get_data(node));
    }
    dlist_destroy(balancer->loop_info_list);
    destroy(balancer);
}

int loop_balancer_attach(loop_balancer_t* balancer, loop_t* loop) {
    dlist_node_t* node      = 0;
    dlist_node_t* temp      = 0;
    loop_info_t*  loop_info = 0;
    int           error     = error_ok;
    verify(balancer);
    verify(loop);
    lock_lock(balancer->lock);
    dlist_for_each_safe(balancer->loop_info_list, node, temp) {
        loop_info = (loop_info_t*)dlist_node_get_data(node);
        if (loop_info->loop == loop) {
            error = error_loop_attached;
            goto unlock_return;
        }
    }
    loop_info = create(loop_info_t);
    verify(loop_info);
    memset(loop_info, 0, sizeof(loop_info_t));
    loop_info->loop = loop;
    dlist_add_tail_node(balancer->loop_info_list, loop_info);
    loop_set_balancer(loop, balancer);
unlock_return:
    lock_unlock(balancer->lock);
    return error;
}

int loop_balancer_detach(loop_balancer_t* balancer, loop_t* loop) {
    dlist_node_t* node      = 0;
    dlist_node_t* found     = 0;
    dlist_node_t* temp      = 0;
    loop_info_t*  loop_info = 0;
    int           error     = error_ok;
    verify(balancer);
    verify(loop);
    lock_lock(balancer->lock);
    dlist_for_each_safe(balancer->loop_info_list, node, temp) {
        loop_info = (loop_info_t*)dlist_node_get_data(node);
        if (loop_info->loop == loop) {
            found = node;
            break;
        }
    }
    if (found) {
        loop_set_balancer(loop_info->loop, 0);
        destroy(loop_info);
        dlist_delete(balancer->loop_info_list, found);
    } else {
        error = error_loop_not_found;
    }
    lock_unlock(balancer->lock);
    return error;
}

loop_t* loop_balancer_choose(loop_balancer_t* balancer) {
    dlist_node_t* node          = 0;
    dlist_node_t* temp          = 0;
    dlist_t*      channel_list  = 0;
    loop_info_t*  found         = 0;
    loop_info_t*  loop_info     = 0;
    int           channel_count = INT_MAX;
    int           count         = 0;
    verify(balancer);
    lock_lock(balancer->lock);
    /* 选取当前活跃管道数最少的loop_t */
    dlist_for_each_safe(balancer->loop_info_list, node, temp) {
        loop_info = (loop_info_t*)dlist_node_get_data(node);
        /* 是否开启loop_balancer_in配置 */
        if (loop_check_balance_options(loop_info->loop, loop_balancer_in)) {
            channel_list = loop_get_active_list(loop_info->loop);
            count = dlist_get_count(channel_list);
            if (count < channel_count) {
                found = loop_info;
                channel_count = count;
            }
        }
    }
    if (found) {
        /* 记录被选取次数，保留 */
        found->choose++;
    }
    lock_unlock(balancer->lock);
    if (found) {
        return found->loop;
    }
    return 0;
}

void loop_balancer_set_data(loop_balancer_t* balancer, void* data) {
    verify(balancer);
    verify(data);
    balancer->data = data;
}

void* loop_balancer_get_data(loop_balancer_t* balancer) {
    verify(balancer);
    return balancer->data;
}
