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
 * ����ѭ��
 */
struct _loop_t {
    kdlist_t*                  active_channel_list; /* ��Ծ�ܵ����� */
    kdlist_t*                  close_channel_list;  /* �ѹرչܵ����� */
    kdlist_t*                  event_list;          /* �߳��¼����� */
    klock_t*                   lock;                /* ��-�߳��¼����� */
    kchannel_ref_t*            notify_channel;      /* �¼�֪ͨд�ܵ� */
    kchannel_ref_t*            read_channel;        /* �¼�֪ͨ���ܵ� */
    kloop_balancer_t*          balancer;            /* ���ؾ����� */
    void*                      impl;                /* �¼�ѡȡ��ʵ�� */
    volatile int               running;             /* �¼�ѭ�����б�־ */
    thread_id_t                thread_id;           /* �¼�ѡȡ����ǰ�����߳�ID */
    knet_loop_balance_option_e balance_options;     /* ���ؾ������� */
    kloop_profile_t*           profile;             /* ͳ�� */
    void*                      data;                /* �û�����ָ�� */
    ktimer_loop_t*             timer_loop;          /* ��ʱ��ѭ�� */
};

/**
 * �����߳��¼�����
 */
typedef enum _loop_event_e {
    loop_event_accept = 1,    /* �����������¼� */
    loop_event_connect,       /* ���������¼� */
    loop_event_send,          /* �����¼� */
    loop_event_close,         /* �ر��¼� */
    loop_event_accept_async,  /* �첽������� */
} loop_event_e;

/**
 * �����߳��¼�
 */
typedef struct _loop_event_t {
    kchannel_ref_t* channel_ref; /* �¼���عܵ� */
    kbuffer_t*      send_buffer; /* ���ͻ�����ָ�� */
    loop_event_e    event;       /* �¼����� */
} loop_event_t;

loop_event_t* loop_event_create(kchannel_ref_t* channel_ref, kbuffer_t* send_buffer, loop_event_e e) {
    loop_event_t* ev = 0;
    verify(channel_ref); /* send_buffer����Ϊ0 */
    ev = create(loop_event_t);
    verify(ev);
    ev->channel_ref = channel_ref;
    ev->send_buffer = send_buffer;
    ev->event       = e;
    return ev;
}

void loop_event_destroy(loop_event_t* loop_event) {
    verify(loop_event);
    knet_free(loop_event);
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
    socket_t pair[2] = {0}; /* �߳��¼���д������ */
    kloop_t* loop    = create(kloop_t);
    verify(loop);
    memset(loop, 0, sizeof(kloop_t));
    /* ����ѡȡ��ʵ�� */
    if (knet_impl_create(loop)) {
        knet_free(loop);
        log_fatal("knet_loop_create() failed, reason: knet_impl_create()");
        return 0;
    }
    /* �����߳��¼���д������ */
    if (socket_pair(pair)) {
        knet_free(loop);
        log_fatal("knet_loop_create() failed, reason: socket_pair()");
        return 0;
    }
    loop->profile             = knet_loop_profile_create(loop);       /* ͳ�� */
    loop->active_channel_list = dlist_create();                       /* ��Ծ�ܵ����� */
    loop->close_channel_list  = dlist_create();                       /* �ӳٹرչܵ����� */
    loop->event_list          = dlist_create();                       /* ���߳��¼����� */
    loop->lock                = lock_create();                        /* �� - ���߳��¼����� */
    loop->timer_loop          = ktimer_loop_create(0);                /* ������ʱ��ѭ�� */
    loop->balance_options     = loop_balancer_in | loop_balancer_out; /* ���ؾ������� */
    loop->notify_channel      = knet_loop_create_channel_exist_socket_fd(loop, pair[0], 0, 0); /* ���߳��¼�֪ͨд�ܵ� */
    verify(loop->notify_channel);
    loop->read_channel = knet_loop_create_channel_exist_socket_fd(loop, pair[1], 0, 1024 * 16); /* ���߳��¼�֪ͨ���ܵ� */
    verify(loop->read_channel);
    /* ���¼��ܵ����뵽��Ծ�ܵ���������Ϊ��Ծ״̬*/
    knet_loop_add_channel_ref(loop, loop->notify_channel);
    knet_loop_add_channel_ref(loop, loop->read_channel);
    knet_channel_ref_set_state(loop->notify_channel, channel_state_active);
    knet_channel_ref_set_state(loop->read_channel, channel_state_active);
    /* ע����¼� */
    knet_channel_ref_set_event(loop->read_channel, channel_event_recv);
    /* ���ö��¼��ص� */
    knet_channel_ref_set_cb(loop->read_channel, knet_loop_queue_cb);
    return loop;
}

void knet_loop_destroy(kloop_t* loop) {
    kdlist_node_t*  node        = 0;
    kdlist_node_t*  temp        = 0;
    kchannel_ref_t* channel_ref = 0;
    loop_event_t*   event       = 0;
    verify(loop);
    /* �ر����л�Ծ�ܵ� */
    dlist_for_each_safe(loop->active_channel_list, node, temp) {
        channel_ref = (kchannel_ref_t*)dlist_node_get_data(node);
        /* �رյĻ�Ծ�ܵ�����Ǩ�Ƶ��ӳٹر����� */
        knet_channel_ref_update_close_in_loop(knet_channel_ref_get_loop(channel_ref), channel_ref);
    }
    /* �����ѹرչܵ� */
    dlist_for_each_safe(loop->close_channel_list, node, temp) {
        channel_ref = (kchannel_ref_t*)dlist_node_get_data(node);
        knet_channel_ref_destroy(channel_ref);
    }
    /* ����ѡȡ������ʵ�� */
    knet_impl_destroy(loop);
    dlist_destroy(loop->close_channel_list); /* ���ٹر����� */
    dlist_destroy(loop->active_channel_list); /* ���ٻ�Ծ���� */
    /* ����δ������߳��¼� */
    dlist_for_each_safe(loop->event_list, node, temp) {
        event = (loop_event_t*)dlist_node_get_data(node);
        knet_free(event);
    }
    /* ����ͳ���� */
    knet_loop_profile_destroy(loop->profile);
    /* �����¼����� */
    dlist_destroy(loop->event_list);
    /* �����¼������� */
    lock_destroy(loop->lock);
    /* ���ٶ�ʱ��ѭ��, ���йܵ���ʱ���������� */
    ktimer_loop_destroy(loop->timer_loop);
    /* ��������ѭ�� */
    knet_free(loop);
}

void loop_add_event(kloop_t* loop, loop_event_t* loop_event) {
    verify(loop);
    verify(loop_event);
    lock_lock(loop->lock); /* �� */
    log_verb("invoke loop_add_event(), event[type:%d]", loop_event->event);
    /* �¼���ӵ�����β�� */
    dlist_add_tail_node(loop->event_list, loop_event);
    lock_unlock(loop->lock); /* ���� */
    knet_loop_notify(loop); /* ֪ͨĿ�� */
}

void knet_loop_notify_accept(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* ���accept�¼� */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_accept));
}

void knet_loop_notify_accept_async(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* ����첽accept�¼� */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_accept_async));
}

void knet_loop_notify_connect(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* ���connect�¼� */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_connect));
}

void knet_loop_notify_send(kloop_t* loop, kchannel_ref_t* channel_ref, kbuffer_t* send_buffer) {
    verify(loop);
    verify(channel_ref);
    verify(send_buffer);
    /* ���send�¼� */
    loop_add_event(loop, loop_event_create(channel_ref, send_buffer, loop_event_send));
}

void knet_loop_notify_close(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* ��ӹر��¼� */
    loop_add_event(loop, loop_event_create(channel_ref, 0, loop_event_close));
}

void knet_loop_queue_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    verify(channel);
    if (e & channel_cb_event_recv) {
        /* ������ж���������, ��Щ����(û��ʵ������)ֻ�Ǵ������¼�����loop���������̷߳��͹������¼� */
        knet_stream_eat_all(knet_channel_ref_get_stream(channel));
        /* һ��ȫ�������������¼� */
        knet_loop_event_process(knet_channel_ref_get_loop(channel));
    } else if (e & channel_cb_event_close) {
        if (knet_loop_check_running(knet_channel_ref_get_loop(channel))) {
            /* ���ӶϿ�, �������� */
            verify(0);
        }
    }
}

void knet_loop_notify(kloop_t* loop) {
    char c = 1;
    verify(loop);
    /* ����һ���ֽڴ������ص�  */
    socket_send(knet_channel_ref_get_socket_fd(loop->notify_channel), &c, sizeof(c));
}

void knet_loop_event_process(kloop_t* loop) {
    /*
     * loop���̵߳Ĵ���ԭ������:
     * 1. �κιܵ�(kchannel_ref_t)�����в���ֻ����һ���߳���, ���ܵ����߳��ǰ󶨵Ĺ�ϵ
     * 2. ���κ�һ���߳��ڲ����ܵ�, ����ܵ����߳�û�а󶨹�ϵ, �������¼���ʽ���������̴߳���
     */
    kdlist_node_t* node       = 0;
    kdlist_node_t* temp       = 0;
    loop_event_t*  loop_event = 0;
    verify(loop);
    lock_lock(loop->lock); /* �� */
    /* ÿ�ζ��¼��ص��ڴ��������¼����� */
    dlist_for_each_safe(loop->event_list, node, temp) {
        loop_event = (loop_event_t*)dlist_node_get_data(node);
        switch(loop_event->event) {
            case loop_event_accept: /* ���������� */
                knet_channel_ref_update_accept_in_loop(loop, loop_event->channel_ref);
                break;
            case loop_event_accept_async: /* ��ǰloop��accept() */
                knet_channel_ref_accept_async(loop_event->channel_ref);
                break;
            case loop_event_connect: /* ��ǰloop��connect */
                knet_channel_ref_connect_in_loop(loop_event->channel_ref);
                break;
            case loop_event_send: /* ��ǰloop��send */
                knet_channel_ref_update_send_in_loop(loop, loop_event->channel_ref, loop_event->send_buffer);
                break;
            case loop_event_close: /* ��ǰloop��close */
                knet_channel_ref_update_close_in_loop(loop, loop_event->channel_ref);
                break;
            default:
                break;
        }
        /* �����¼� */
        loop_event_destroy(loop_event);
        /* ��������ڵ� */
        dlist_delete(loop->event_list, node);
    }
    lock_unlock(loop->lock); /* ���� */
}

kchannel_ref_t* knet_loop_create_channel_exist_socket_fd(kloop_t* loop, socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    verify(loop);
    return knet_channel_ref_create(loop, knet_channel_create_exist_socket_fd(socket_fd, max_send_list_len, recv_ring_len, 0));
}

kchannel_ref_t* knet_loop_create_channel_exist_socket_fd6(kloop_t* loop, socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    verify(loop);
    return knet_channel_ref_create(loop, knet_channel_create_exist_socket_fd(socket_fd, max_send_list_len, recv_ring_len, 1));
}

kchannel_ref_t* knet_loop_create_channel(kloop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    verify(loop);
    return knet_channel_ref_create(loop, knet_channel_create(max_send_list_len, recv_ring_len, 0));
}

kchannel_ref_t* knet_loop_create_channel6(kloop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    verify(loop);
    return knet_channel_ref_create(loop, knet_channel_create(max_send_list_len, recv_ring_len, 1));
}

thread_id_t knet_loop_get_thread_id(kloop_t* loop) {
    verify(loop);
    return loop->thread_id;
}

int knet_loop_run_once(kloop_t* loop) {
    verify(loop);
    /* ��ȡ��ǰ�߳�ID */
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
        /* �Ѿ��������������Ҫ��������ڵ� */
        dlist_add_front(loop->active_channel_list, node);
    } else {
        /* ��������ڵ� */
        dlist_add_front_node(loop->active_channel_list, channel_ref);
    }
    knet_loop_profile_decrease_active_channel_count(loop->profile);
    knet_loop_profile_increase_established_channel_count(loop->profile);
    /* ���ýڵ� */
    knet_channel_ref_set_loop_node(channel_ref, dlist_get_front(loop->active_channel_list));
    /* ֪ͨѡȡ����ӹܵ� */
    knet_impl_add_channel_ref(loop, channel_ref);
}

void knet_loop_remove_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* ����뵱ǰ����������������ٽڵ� */
    dlist_remove(loop->active_channel_list, knet_channel_ref_get_loop_node(channel_ref));
    /* ͳ����Ϣ */
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
    /* ����㺯�����׽����Ѿ����ر� */
    /* �ӻ�Ծ������ȡ�� */
    knet_loop_remove_channel_ref(loop, channel_ref);
    /* ���뵽�ѹر����� */
    dlist_add_front(loop->close_channel_list, knet_channel_ref_get_loop_node(channel_ref));
}

void knet_loop_set_balancer(kloop_t* loop, kloop_balancer_t* balancer) {
    verify(loop); /* balancer����Ϊ0 */
    loop->balancer = balancer;
}

kloop_balancer_t* knet_loop_get_balancer(kloop_t* loop) {
    verify(loop);
    return loop->balancer;
}

void knet_loop_check_timeout(kloop_t* loop, time_t ts) {
    (void)ts;
    /* ���ܵ���ʱ, �������ӳ�ʱ�Ͷ���ʱ */
    ktimer_loop_run_once(loop->timer_loop);
}

void knet_loop_check_close(kloop_t* loop) {
    kdlist_node_t*  node        = 0;
    kdlist_node_t*  temp        = 0;
    kchannel_ref_t* channel_ref = 0;
    verify(loop);
    dlist_for_each_safe(knet_loop_get_close_list(loop), node, temp) {
        channel_ref = (kchannel_ref_t*)dlist_node_get_data(node);
        /* �ڶ��̻߳�����, ���ڶ��̳߳��йܵ����õ������ü�����Ϊ��, ��֤�û��ص��ڱ��߳�ֻ�ᱻ����һ�� */
        if (!knet_channel_ref_check_close_cb_called(channel_ref)) {
            /* �����û��ص� */
            if (knet_channel_ref_get_cb(channel_ref)) {
                knet_channel_ref_get_cb(channel_ref)(channel_ref, channel_cb_event_close);
            }
            /* ���ùر��¼��ص���־ */
            knet_channel_ref_set_close_cb_called(channel_ref);
        }
        /* ���ٹܵ� */
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
