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

#include "framework_raiser.h"
#include "loop.h"
#include "loop_balancer.h"
#include "framework.h"
#include "channel_ref.h"
#include "framework_config.h"
#include "list.h"
#include "misc.h"

int _create_acceptor_channel(framework_acceptor_config_t* ac, loop_t* loop);
int _create_connector_channel(framework_connector_config_t* cc, loop_t* loop);

struct _framework_raiser_t {
    loop_t*          loop;   /* 网络事件循环 */
    framework_t*     f;      /* 框架 */
    thread_runner_t* runner; /* 线程 */
};

framework_raiser_t* framework_raiser_create(framework_t* f, loop_t* loop) {    
    framework_raiser_t* raiser = 0;
    verify(f);
    raiser = create(framework_raiser_t);
    verify(raiser);
    memset(raiser, 0, sizeof(framework_raiser_t));
    raiser->f    = f;
    raiser->loop = loop;
    verify(raiser->loop);
    /* 所有接受/发起连接的新管道全部给其他loop_t处理 */
    loop_set_balance_options(raiser->loop, loop_balancer_out);
    return raiser;
}

void framework_raiser_destroy(framework_raiser_t* raiser) {
    verify(raiser);
    if (raiser->runner) {
        if (thread_runner_check_start(raiser->runner)) {
            thread_runner_stop(raiser->runner);
        }
        thread_runner_destroy(raiser->runner);
    }
    destroy(raiser);
}

int _create_acceptor_channel(framework_acceptor_config_t* ac, loop_t* loop) {
    int            error   = error_ok;
    channel_ref_t* channel = 0;
    /* 建立监听管道 */
    channel = loop_create_channel(loop, framework_acceptor_config_get_client_max_send_list_count(ac),
        framework_acceptor_config_get_client_max_recv_buffer_length(ac));
    verify(channel);
    channel_ref_set_cb(channel, acceptor_cb);
    channel_ref_set_user_data(channel, ac);
    /* 监听 */
    error = channel_ref_accept(channel, framework_acceptor_config_get_ip(ac),
        framework_acceptor_config_get_port(ac), framework_acceptor_config_get_backlog(ac));
    if (error != error_ok) {
        channel_ref_destroy(channel);
    }
    return error;
}

int _create_connector_channel(framework_connector_config_t* cc, loop_t* loop) {
    int            error   = error_ok;
    channel_ref_t* channel = 0;
    /* 建立监听管道 */
    channel = loop_create_channel(loop, framework_connector_config_get_max_send_list_count(cc),
        framework_connector_config_get_max_recv_buffer_length(cc));
    verify(channel);
    channel_ref_set_cb(channel, framework_connector_config_get_cb(cc));
    /* 监听 */
    error = channel_ref_connect(channel, framework_connector_config_get_remote_ip(cc),
        framework_connector_config_get_remote_port(cc), framework_connector_config_get_connect_timeout(cc));
    if (error != error_ok) {
        channel_ref_destroy(channel);
    }
    return error;
}

int framework_raiser_start(framework_raiser_t* raiser) {
    int                           error            = 0;
    framework_config_t*           config           = 0;
    framework_acceptor_config_t*  ac               = 0;
    framework_connector_config_t* cc               = 0;
    dlist_node_t*                 node             = 0;
    dlist_node_t*                 temp             = 0;
    dlist_t*                      acceptor_config  = 0;
    dlist_t*                      connector_config = 0;
    verify(raiser);
    config  = framework_get_config(raiser->f);
    /* 关联到负载均衡器，监听器所有新连接都会被分派到工作线程处理 */
    loop_balancer_attach(framework_get_balancer(raiser->f), raiser->loop);
    verify(config);
    /* 建立监听管道 */
    acceptor_config = framework_config_get_acceptor_config(config);
    dlist_for_each_safe(acceptor_config, node, temp) {
        ac = (framework_acceptor_config_t*)dlist_node_get_data(node);
        error = _create_acceptor_channel(ac, raiser->loop);
        if (error_ok != error) {
            return error;
        }
    }
    /* 建立连接器管道 */
    connector_config = framework_config_get_connector_config(config);
    dlist_for_each_safe(connector_config, node, temp) {
        cc = (framework_connector_config_t*)dlist_node_get_data(node);
        error = _create_connector_channel(cc, raiser->loop);
        if (error_ok != error) {
            return error;
        }
    }
    raiser->runner = thread_runner_create(0, 0);
    verify(raiser->runner);
    /* 启动网络事件循环 */
    return thread_runner_start_loop(raiser->runner, raiser->loop, 0);
}

void framework_raiser_stop(framework_raiser_t* raiser) {
    verify(raiser);
    if (raiser->runner) {
        if (thread_runner_check_start(raiser->runner)) {
            thread_runner_stop(raiser->runner);
        }
    }
}

void framework_raiser_wait_for_stop(framework_raiser_t* raiser) {
    verify(raiser);
    if (raiser->runner) {
        thread_runner_join(raiser->runner);
    }
}

void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    framework_acceptor_config_t* ac = (framework_acceptor_config_t*)channel_ref_get_user_data(channel);
    channel_ref_cb_t             cb = framework_acceptor_config_get_client_cb(ac);
    if (e & channel_cb_event_accept) {
        /* 设置用户回调 */
        channel_ref_set_cb(channel, cb);
        /* 设置心跳间隔 */
        channel_ref_set_timeout(channel,
            framework_acceptor_config_get_client_heartbeat_timeout(ac));
        if (cb) {
            /* 调用一次用户回调，在用户回调内通知新连接建立 */
            cb(channel, e);
        }
    }
}
