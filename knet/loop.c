/*
 * Copyright (c) 2014-2016, dennis wang
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

#include "config.h"
#include "loop.h"
#include "list.h"
#include "channel_ref.h"
#include "channel.h"
#include "misc.h"
#include "loop_balancer.h"
#include "loop_profile.h"
#include "stream.h"
#include "logger.h"
#include "timer.h"

/**
 * 网络循环
 */
struct _loop_t {
    kdlist_t*                  active_channel_list; /* 活跃管道链表 */
    kdlist_t*                  close_channel_list;  /* 已关闭管道链表 */
    kdlist_t*                  event_list;          /* 线程事件链表 */
    klock_t*                   lock;                /* 锁-线程事件链表 */
    kchannel_ref_t*            notify_channel;      /* 事件通知写管道 */
    kchannel_ref_t*            read_channel;        /* 事件通知读管道 */
    kloop_balancer_t*          balancer;            /* 负载均衡器 */
    void*                      impl;                /* 事件选取器实现 */
    volatile int               running;             /* 事件循环运行标志 */
    thread_id_t                thread_id;           /* 事件选取器当前运行线程ID */
    knet_loop_balance_option_e balance_options;     /* 负载均衡配置 */
    kloop_profile_t*           profile;             /* 统计 */
    void*                      data;                /* 用户数据指针 */
    ktimer_loop_t*             timer_loop;          /* 定时器循环 */
};

/**
 * 网络线程事件类型
 */
typedef enum _loop_event_e {
    loop_event_accept = 1,    /* 接受新连接事件 */
    loop_event_connect,       /* 发起连接事件 */
    loop_event_send,          /* 发送事件 */
    loop_event_close,         /* 关闭事件 */
    loop_event_accept_async,  /* 异步发起监听 */
} loop_event_e;

/**
 * 网络线程事件
 */
typedef struct _loop_event_t {
    kchannel_ref_t* channel_ref; /* 事件相关管道 */
    kbuffer_t*      send_buffer; /* 发送缓冲区指针 */
    loop_event_e    event;       /* 事件类型 */
} loop_event_t;

loop_event_t* loop_event_create(kchannel_ref_t* channel_ref, kbuffer_t* send_buffer, loop_event_e e) {
    loop_event_t* ev = 0;
    verify(channel_ref); /* send_buffer可以为0 */
    ev = create(loop_event_t);
    verify(ev);
    ev->channel_ref = channel_ref;
    ev->send_buffer = send_buffer;
    ev->event       = e;
    return ev;
}

void loop_event_destroy(loop_event_t* loop_event) {
    verify(loop_event);
    destroy(loop_event);
}

kchannel_ref_t* loop_event_get_channel_ref(loop_event_t* loop_event) {
    verify(loop_event);
    return loop_event->channel_ref;
}

kbuffer_t* loop_event_get_send_buffer(loop_event_t* loop_event) {
    verify(loop_event);
    return loop_event->send_buffer;
}

loop_event_e loop_event_get_event(loop_event_t* loop_event) {
    verify(loop_event);
    return loop_event->event;
}

kloop_t* knet_loop_create() {
    socket_t pair[2] = {0}; /* 线程事件读写描述符 */
    kloop_t* loop    = create(kloop_t);
    verify(loop);
    memset(loop, 0, sizeof(kloop_t));
    /* 建立选取器实现 */
    if (knet_impl_create(loop)) {
        destroy(loop);
        log_fatal("knet_loop_create() failed, reason: knet_impl_create()");
        return 0;
    }
    /* 建立线程事件读写描述符 */
    if (socket_pair(pair)) {
        destroy(loop);
        log_fatal("knet_loop_create() failed, reason: socket_pair()");
        return 0;
    }
    loop->profile             = knet_loop_profile_create(loop);       /* 统计 */
    loop->active_channel_list = dlist_create();                       /* 活跃管道链表 */
    loop->close_channel_list  = dlist_create();                       /* 延迟关闭管道链表 */
    loop->event_list          = dlist_create();                       /* 跨线程事件链表 */
    loop->lock                = lock_create();                        /* 锁 - 跨线程事件链表 */
    loop->timer_loop          = ktimer_loop_create(0);                /* 建立定时器循环 */
    loop->balance_options     = loop_balancer_in | loop_balancer_out; /* 负载均衡配置 */
    loop->notify_channel      = knet_loop_create_channel_exist_socket_fd(loop, pair[0], 0, 0); /* 跨线程事件通知写管道 */
    verify(loop->notify_channel);
    loop->read_channel = knet_loop_create_channel_exist_socket_fd(loop, pair[1], 0, 1024 * 16); /* 跨线程事件通知读管道 */
    verify(loop->read_channel);
    /* 将事件管道加入到活跃管道链并设置为活跃状态*/
    knet_loop_add_channel_ref(loop, loop->notify_channel);
    knet_loop_add_channel_ref(loop, loop->read_channel);
    knet_channel_ref_set_state(loop->notify_channel, channel_state_active);
    knet_channel_ref_set_state(loop->read_channel, channel_state_active);
    /* 注册读事件 */
    knet_channel_ref_set_event(loop->read_channel, channel_event_recv);
    /* 设置读事件回调 */
    knet_channel_ref_set_cb(loop->read_channel, knet_loop_queue_cb);
    return loop;
}

void knet_loop_destroy(kloop_t* loop) {
    kdlist_node_t*  node        = 0;
    kdlist_node_t*  temp        = 0;
    kchannel_ref_t* channel_ref = 0;
    loop_event_t*   event       = 0;
    verify(loop);
    /* 关闭所有活跃管道 */
    dlist_for_each_safe(loop->active_channel_list, node, temp) {
        channel_ref = (kchannel_ref_t*)dlist_node_get_data(node);
        /* 关闭的活跃管道将被迁移到延迟关闭链表 */
        knet_channel_ref_update_close_in_loop(knet_channel_ref_get_loop(channel_ref), channel_ref);
    }
    /* 销毁已关闭管道 */
    dlist_for_each_safe(loop->close_channel_list, node, temp) {
        channel_ref = (kchannel_ref_t*)dlist_node_get_data(node);
        knet_channel_ref_destroy(channel_ref);
    }
    /* 销毁选取器本地实现 */
    knet_impl_destroy(loop);
    dlist_destroy(loop->close_channel_list); /* 销毁关闭链表 */
    dlist_destroy(loop->active_channel_list); /* 销毁活跃链表 */
    /* 销毁未处理跨线程事件 */
    dlist_for_each_safe(loop->event_list, node, temp) {
        event = (loop_event_t*)dlist_node_get_data(node);
        destroy(event);
    }
    /* 销毁统计器 */
    knet_loop_profile_destroy(loop->profile);
    /* 销毁事件链表 */
    dlist_destroy(loop->event_list);
    /* 销毁事件链表锁 */
    lock_destroy(loop->lock);
    /* 销毁定时器循环, 所有管道定时器将被销毁 */
    ktimer_loop_destroy(loop->timer_loop);
    /* 销毁网络循环 */
    destroy(loop);
}

void loop_add_event(kloop_t* loop, loop_event_t* loop_event) {
    verify(loop);
    verify(loop_event);
    lock_lock(loop->lock); /* 锁 */
    log_verb("invoke loop_add_event(), event[type:%d]", loop_event->event);
    /* 事件添加到链表尾部 */
    dlist_add_tail_node(loop->event_list, loop_event);
    lock_unlock(loop->lock); /* 解锁 */
    knet_loop_notify(loop); /* 通知目标 */
}

void knet_loop_notify_accept(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* 添加accept事件 */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_accept));
}

void knet_loop_notify_accept_async(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* 添加异步accept事件 */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_accept_async));
}

void knet_loop_notify_connect(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* 添加connect事件 */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_connect));
}

void knet_loop_notify_send(kloop_t* loop, kchannel_ref_t* channel_ref, kbuffer_t* send_buffer) {
    verify(loop);
    verify(channel_ref);
    verify(send_buffer);
    /* 添加send事件 */
    loop_add_event(loop, loop_event_create(channel_ref, send_buffer, loop_event_send));
}

void knet_loop_notify_close(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* 添加关闭事件 */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_close));
}

void knet_loop_queue_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    verify(channel);
    if (e & channel_cb_event_recv) {
        /* 清空所有读到的数据, 这些数据(没有实际意义)只是触发读事件来让loop处理其他线程发送过来的事件 */
        knet_stream_eat_all(knet_channel_ref_get_stream(channel));
        /* 一次全部处理完所有事件 */
        knet_loop_event_process(knet_channel_ref_get_loop(channel));
    } else if (e & channel_cb_event_close) {
        if (knet_loop_check_running(knet_channel_ref_get_loop(channel))) {
            /* 连接断开, 致命错误 */
            verify(0);
        }
    }
}

void knet_loop_notify(kloop_t* loop) {
    char c = 1;
    verify(loop);
    /* 发送一个字节触发读回调  */
    socket_send(knet_channel_ref_get_socket_fd(loop->notify_channel), &c, sizeof(c));
}

void knet_loop_event_process(kloop_t* loop) {
    /*
     * loop多线程的处理原则如下:
     * 1. 任何管道(kchannel_ref_t)的所有操作只会在一个线程内, 即管道与线程是绑定的关系
     * 2. 在任何一个线程内操作管道, 如果管道与线程没有绑定关系, 都会以事件方式发送至绑定线程处理
     */
    kdlist_node_t* node       = 0;
    kdlist_node_t* temp       = 0;
    loop_event_t*  loop_event = 0;
    verify(loop);
    lock_lock(loop->lock); /* 锁 */
    /* 每次读事件回调内处理整个事件链表 */
    dlist_for_each_safe(loop->event_list, node, temp) {
        loop_event = (loop_event_t*)dlist_node_get_data(node);
        switch(loop_event->event) {
            case loop_event_accept: /* 接受新连接 */
                knet_channel_ref_update_accept_in_loop(loop, loop_event->channel_ref);
                break;
            case loop_event_accept_async: /* 当前loop内accept() */
                knet_channel_ref_accept_async(loop_event->channel_ref);
                break;
            case loop_event_connect: /* 当前loop内connect */
                knet_channel_ref_connect_in_loop(loop_event->channel_ref);
                break;
            case loop_event_send: /* 当前loop内send */
                knet_channel_ref_update_send_in_loop(loop, loop_event->channel_ref, loop_event->send_buffer);
                break;
            case loop_event_close: /* 当前loop内close */
                knet_channel_ref_update_close_in_loop(loop, loop_event->channel_ref);
                break;
            default:
                break;
        }
        /* 销毁事件 */
        loop_event_destroy(loop_event);
        /* 销毁链表节点 */
        dlist_delete(loop->event_list, node);
    }
    lock_unlock(loop->lock); /* 解锁 */
}

kchannel_ref_t* knet_loop_create_channel_exist_socket_fd(kloop_t* loop, socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    verify(loop);
    return knet_channel_ref_create(loop, knet_channel_create_exist_socket_fd(socket_fd, max_send_list_len, recv_ring_len));
}

kchannel_ref_t* knet_loop_create_channel(kloop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    verify(loop);
    return knet_channel_ref_create(loop, knet_channel_create(max_send_list_len, recv_ring_len));
}

thread_id_t knet_loop_get_thread_id(kloop_t* loop) {
    verify(loop);
    return loop->thread_id;
}

int knet_loop_run_once(kloop_t* loop) {
    verify(loop);
    /* 获取当前线程ID */
    loop->thread_id = thread_get_self_id();
    return knet_impl_run_once(loop);
}

int knet_loop_run(kloop_t* loop) {
    int error = 0;
    verify(loop);
    loop->running = 1;
    while (loop->running) {
        error = knet_loop_run_once(loop);
        if (error_loop_fail == error) {
            loop->running = 0;
        }
    }
    return error;
}

void knet_loop_exit(kloop_t* loop) {
    verify(loop);
    loop->running = 0;
}

kdlist_t* knet_loop_get_active_list(kloop_t* loop) {
    verify(loop);
    return loop->active_channel_list;
}

kdlist_t* knet_loop_get_close_list(kloop_t* loop) {
    verify(loop);
    return loop->close_channel_list;
}

void knet_loop_add_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    kdlist_node_t* node  = 0;
    verify(channel_ref);
    verify(loop);
    node = knet_channel_ref_get_loop_node(channel_ref);
    if (node) {
        /* 已经加入过链表，不需要创建链表节点 */
        dlist_add_front(loop->active_channel_list, node);
    } else {
        /* 创建链表节点 */
        dlist_add_front_node(loop->active_channel_list, channel_ref);
    }
    knet_loop_profile_decrease_active_channel_count(loop->profile);
    knet_loop_profile_increase_established_channel_count(loop->profile);
    /* 设置节点 */
    knet_channel_ref_set_loop_node(channel_ref, dlist_get_front(loop->active_channel_list));
    /* 通知选取器添加管道 */
    knet_impl_add_channel_ref(loop, channel_ref);
}

void knet_loop_remove_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* 解除与当前链表关联，但不销毁节点 */
    dlist_remove(loop->active_channel_list, knet_channel_ref_get_loop_node(channel_ref));
    /* 统计信息 */
    knet_loop_profile_decrease_established_channel_count(loop->profile);
    knet_loop_profile_increase_close_channel_count(loop->profile);
}

void knet_loop_set_impl(kloop_t* loop, void* impl) {
    verify(loop);
    verify(impl);
    loop->impl = impl;
}

void* knet_loop_get_impl(kloop_t* loop) {
    verify(loop);
    return loop->impl;
}

void knet_loop_close_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* 在外层函数内套接字已经被关闭 */
    /* 从活跃链表内取出 */
    knet_loop_remove_channel_ref(loop, channel_ref);
    /* 加入到已关闭链表 */
    dlist_add_front(loop->close_channel_list, knet_channel_ref_get_loop_node(channel_ref));
}

void knet_loop_set_balancer(kloop_t* loop, kloop_balancer_t* balancer) {
    verify(loop); /* balancer可以为0 */
    loop->balancer = balancer;
}

kloop_balancer_t* knet_loop_get_balancer(kloop_t* loop) {
    verify(loop);
    return loop->balancer;
}

void knet_loop_check_timeout(kloop_t* loop, time_t ts) {
    (void)ts;
    /* 检测管道超时, 包含连接超时和读超时 */
    ktimer_loop_run_once(loop->timer_loop);
}

void knet_loop_check_close(kloop_t* loop) {
    kdlist_node_t*  node        = 0;
    kdlist_node_t*  temp        = 0;
    kchannel_ref_t* channel_ref = 0;
    verify(loop);
    dlist_for_each_safe(knet_loop_get_close_list(loop), node, temp) {
        channel_ref = (kchannel_ref_t*)dlist_node_get_data(node);
        /* 在多线程环境下, 由于多线程持有管道引用导致引用计数不为零, 保证用户回调在本线程只会被调用一次 */
        if (!knet_channel_ref_check_close_cb_called(channel_ref)) {
            /* 调用用户回调 */
            if (knet_channel_ref_get_cb(channel_ref)) {
                knet_channel_ref_get_cb(channel_ref)(channel_ref, channel_cb_event_close);
            }
            /* 设置关闭事件回调标志 */
            knet_channel_ref_set_close_cb_called(channel_ref);
        }
        /* 销毁管道 */
        if (error_ok == knet_channel_ref_destroy(channel_ref)) {
            knet_loop_profile_decrease_close_channel_count(loop->profile);
            dlist_delete(knet_loop_get_close_list(loop), node);
        }
    }
}

int knet_loop_check_running(kloop_t* loop) {
    verify(loop);
    return loop->running;
}

int knet_loop_get_active_channel_count(kloop_t* loop) {
    verify(loop);
    return dlist_get_count(loop->active_channel_list);
}

int knet_loop_get_close_channel_count(kloop_t* loop) {
    verify(loop);
    return dlist_get_count(loop->close_channel_list);
}

void knet_loop_set_balance_options(kloop_t* loop, knet_loop_balance_option_e options) {
    verify(loop);
    loop->balance_options = options;
}

knet_loop_balance_option_e knet_loop_get_balance_options(kloop_t* loop) {
    verify(loop);
    return loop->balance_options;
}

int knet_loop_check_balance_options(kloop_t* loop, knet_loop_balance_option_e options) {
    verify(loop);
    return (loop->balance_options & options);
}

void knet_loop_set_data(kloop_t* loop, void* data) {
    verify(loop);
    loop->data = data;
}

void* knet_loop_get_data(kloop_t* loop) {
    verify(loop);
    return loop->data;
}

kloop_profile_t* knet_loop_get_profile(kloop_t* loop) {
    verify(loop);
    return loop->profile;
}

ktimer_loop_t* knet_loop_get_timer_loop(kloop_t* loop) {
    verify(loop);
    return loop->timer_loop;
}
