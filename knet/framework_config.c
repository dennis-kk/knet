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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "framework_config.h"
#include "misc.h"
#include "list.h"
#include "logger.h"

struct _framework_acceptor_config_t {
    char                  ip[32];                 /* IP */
    int                   port;                   /* 监听端口 */
    int                   backlog;                /* listen() backlog */
    int                   idle_timeout;           /* 心跳（秒） */
    int                   max_send_list_count;    /* 发送链表最大长度 */
    int                   max_recv_buffer_length; /* 接收缓冲区最大长度 */
    knet_channel_ref_cb_t cb;                     /* 回调 */
    void*                 user_data;              /* 用户数据指针 */
};

struct _framework_connector_config_t {
    char                  ip[32];                 /* IP */
    int                   port;                   /* 监听端口 */
    int                   idle_timeout;           /* 心跳（秒） */
    int                   connect_timeout;        /* 连接超时 */
    int                   max_send_list_count;    /* 发送链表最大长度 */
    int                   max_recv_buffer_length; /* 接收缓冲区最大长度 */
    int                   auto_reconnect;         /* 自动重连标志 */
    knet_channel_ref_cb_t cb;                     /* 回调 */
    void*                 user_data;              /* 用户数据指针 */
};

struct _framework_config_t {
    klock_t*  lock;                  /* 锁 */
    kdlist_t* acceptor_config_list;  /* 监听器配置 */
    kdlist_t* connector_config_list; /* 连接器配置 */
    int      worker_thread_count;   /* 工作线程数量 */
    time_t   worker_timer_intval;   /* 定时器（工作线程内）分辨率（毫秒） */
    int      worker_timer_slot;     /* 定时器（工作线程内）时间轮槽位数量 */
};


kframework_config_t* framework_config_create() {
    kframework_config_t* c = create(kframework_config_t);
    verify(c);
    memset(c, 0, sizeof(kframework_config_t));
    c->acceptor_config_list = dlist_create();
    verify(c->acceptor_config_list);
    c->connector_config_list = dlist_create();
    verify(c->connector_config_list);
    c->worker_thread_count = 1;    /* 默认 - 只有一个工作线程 */
    c->worker_timer_intval = 1000; /* 默认 - 分辨率为1000毫秒（1秒） */
    c->worker_timer_slot   = 512;  /* 默认 - 512个槽位 */
    c->lock = lock_create();
    return c;
}

void framework_config_destroy(kframework_config_t* c) {
    kdlist_node_t* node = 0;
    kdlist_node_t* temp = 0;
    verify(c);
    dlist_for_each_safe(c->acceptor_config_list, node, temp) {
        destroy(dlist_node_get_data(node));
    }
    dlist_for_each_safe(c->connector_config_list, node, temp) {
        destroy(dlist_node_get_data(node));
    }
    dlist_destroy(c->acceptor_config_list);
    dlist_destroy(c->connector_config_list);
    lock_destroy(c->lock);
    destroy(c);
}

int knet_framework_config_load_config_file(kframework_config_t* c, const char* path) {
    verify(c);
    verify(path);
    return error_ok;
}

void knet_framework_config_set_worker_thread_count(kframework_config_t* c, int worker_thread_count) {
    verify(c);
    verify(worker_thread_count);
    c->worker_thread_count = worker_thread_count;
}

void knet_framework_config_set_worker_timer_freq(kframework_config_t* c, time_t freq) {
    verify(c);
    if (!freq) {
        freq = 1000;
    }
    c->worker_timer_intval = freq;
}

void knet_framework_config_set_worker_timer_slot(kframework_config_t* c, int slot) {
    verify(c);
    if (slot <= 0) {
        slot = 360;
    }
    c->worker_timer_slot = slot;
}

kframework_acceptor_config_t* knet_framework_config_new_acceptor(kframework_config_t* c) {
    kframework_acceptor_config_t* ac = 0;
    verify(c);
    ac = create(kframework_acceptor_config_t);
    verify(ac);
    memset(ac, 0, sizeof(kframework_acceptor_config_t));
    ac->backlog = 100;
    ac->max_send_list_count    = 128;
    ac->max_recv_buffer_length = 1024 * 16;
    lock_lock(c->lock);
    dlist_add_tail_node(c->acceptor_config_list, ac);
    lock_unlock(c->lock);
    return ac;
}

kframework_connector_config_t* knet_framework_config_new_connector(kframework_config_t* c) {
    kframework_connector_config_t* cc = 0;
    verify(c);
    cc = create(kframework_connector_config_t);
    verify(cc);
    memset(cc, 0, sizeof(kframework_connector_config_t));
    cc->max_send_list_count    = 128;
    cc->max_recv_buffer_length = 1024 * 16;
    lock_lock(c->lock);
    dlist_add_tail_node(c->connector_config_list, cc);
    lock_unlock(c->lock);
    return cc;
}

void knet_framework_acceptor_config_set_local_address(kframework_acceptor_config_t* c, const char* ip, int port) {
    verify(c);
    verify(port);
    if (ip) {
        strcpy(c->ip, ip);
    } else {
        strcpy(c->ip, "0.0.0.0");
    }
    c->port = port;
}

void knet_framework_acceptor_config_set_backlog(kframework_acceptor_config_t* c, int backlog) {
    verify(c);
    if (!backlog) {
        backlog = 100;
    }
    c->backlog = backlog;
}

void knet_framework_acceptor_config_set_client_heartbeat_timeout(kframework_acceptor_config_t* c, int timeout) {
    verify(c);
    c->idle_timeout = timeout;
}

void knet_framework_acceptor_config_set_client_cb(kframework_acceptor_config_t* c, knet_channel_ref_cb_t cb) {
    verify(c);
    c->cb = cb;
}

void knet_framework_acceptor_config_set_client_max_send_list_count(kframework_acceptor_config_t* c, int max_send_list_count) {
    verify(c);
    c->max_send_list_count = max_send_list_count;
}

void knet_framework_acceptor_config_set_client_max_recv_buffer_length(kframework_acceptor_config_t* c, int max_recv_buffer_length) {
    verify(c);
    c->max_recv_buffer_length = max_recv_buffer_length;
}

void knet_framework_connector_config_set_remote_address(kframework_connector_config_t* c, const char* ip, int port) {
    verify(c);
    verify(port);
    if (ip) {
        strcpy(c->ip, ip);
    } else {
        strcpy(c->ip, "0.0.0.0");
    }
    c->port = port;
}

void knet_framework_connector_config_set_heartbeat_timeout(kframework_connector_config_t* c, int timeout) {
    verify(c);
    c->idle_timeout = timeout;
}

void knet_framework_connector_config_set_connect_timeout(kframework_connector_config_t* c, int timeout) {
    verify(c);
    c->connect_timeout = timeout;
}

void knet_framework_connector_config_set_auto_reconnect(kframework_connector_config_t* c, int auto_reconnect) {
    verify(c);
    c->auto_reconnect = auto_reconnect;
}

void knet_framework_connector_config_set_cb(kframework_connector_config_t* c, knet_channel_ref_cb_t cb) {
    verify(c);
    c->cb = cb;
}

void knet_framework_connector_config_set_client_max_send_list_count(kframework_connector_config_t* c, int max_send_list_count) {
    verify(c);
    c->max_send_list_count = max_send_list_count;
}

void knet_framework_connector_config_set_client_max_recv_buffer_length(kframework_connector_config_t* c, int max_recv_buffer_length) {
    verify(c);
    c->max_recv_buffer_length = max_recv_buffer_length;
}

int framework_config_get_worker_thread_count(kframework_config_t* c) {
    verify(c);
    return c->worker_thread_count;
}

time_t framework_config_get_worker_timer_freq(kframework_config_t* c) {
    verify(c);
    return c->worker_timer_intval;
}

int framework_config_get_worker_timer_slot(kframework_config_t* c) {
    verify(c);
    return c->worker_timer_slot;
}

kdlist_t* framework_config_get_acceptor_config(kframework_config_t* c) {
    verify(c);
    return c->acceptor_config_list;
}

kdlist_t* framework_config_get_connector_config(kframework_config_t* c) {
    verify(c);
    return c->connector_config_list;
}

const char* framework_acceptor_config_get_ip(kframework_acceptor_config_t* c) {
    verify(c);
    return c->ip;
}

int framework_acceptor_config_get_port(kframework_acceptor_config_t* c) {
    verify(c);
    return c->port;
}

int framework_acceptor_config_get_backlog(kframework_acceptor_config_t* c) {
    verify(c);
    return c->backlog;
}

int framework_acceptor_config_get_client_heartbeat_timeout(kframework_acceptor_config_t* c) {
    verify(c);
    return c->idle_timeout;
}

knet_channel_ref_cb_t framework_acceptor_config_get_client_cb(kframework_acceptor_config_t* c) {
    verify(c);
    return c->cb;
}

int framework_acceptor_config_get_client_max_send_list_count(kframework_acceptor_config_t* c) {
    verify(c);
    return c->max_send_list_count;
}

int framework_acceptor_config_get_client_max_recv_buffer_length(kframework_acceptor_config_t* c) {
    verify(c);
    return c->max_recv_buffer_length;
}

const char* framework_connector_config_get_remote_ip(kframework_connector_config_t* c) {
    verify(c);
    return c->ip;
}

int framework_connector_config_get_remote_port(kframework_connector_config_t* c) {
    verify(c);
    return c->port;
}

int framework_connector_config_get_heartbeat_timeout(kframework_connector_config_t* c) {
    verify(c);
    return c->idle_timeout;
}

int framework_connector_config_get_connect_timeout(kframework_connector_config_t* c) {
    verify(c);
    return c->connect_timeout;
}

int framework_connector_config_get_auto_reconnect(kframework_connector_config_t* c) {
    verify(c);
    return c->auto_reconnect;
}

knet_channel_ref_cb_t framework_connector_config_get_cb(kframework_connector_config_t* c) {
    verify(c);
    return c->cb;
}

int framework_connector_config_get_max_send_list_count(kframework_connector_config_t* c) {
    verify(c);
    return c->max_send_list_count;
}

int framework_connector_config_get_max_recv_buffer_length(kframework_connector_config_t* c) {
    verify(c);
    return c->max_recv_buffer_length;
}

void framework_acceptor_config_set_user_data(kframework_acceptor_config_t* c, void* user_data) {
    verify(c);
    c->user_data = user_data;
}

void* framework_acceptor_config_get_user_data(kframework_acceptor_config_t* c) {
    verify(c);
    return c->user_data;
}

void framework_connector_config_set_user_data(kframework_connector_config_t* c, void* user_data) {
    verify(c);
    c->user_data = user_data;
}

void* framework_connector_config_get_user_data(kframework_connector_config_t* c) {
    verify(c);
    return c->user_data;
}
