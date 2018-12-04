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

#include "channel_ref.h"
#include "channel.h"
#include "loop.h"
#include "misc.h"
#include "stream.h"
#include "loop_balancer.h"
#include "buffer.h"
#include "ringbuffer.h"
#include "address.h"
#include "loop_profile.h"
#include "logger.h"
#include "timer.h"

/**
 * �ܵ���Ϣ
 */
typedef struct _channel_ref_info_t {
    /* �������ݳ�Ա */
    int                           balance;              /* �Ƿ񱻸��ؾ����־ */
    kchannel_t*                   channel;              /* �ڲ��ܵ� */
    kdlist_node_t*                loop_node;            /* �ܵ�����ڵ�, ����˽ڵ���Բ�������������� */
    kstream_t*                    stream;               /* �ܵ�(��/д)������ */
    kloop_t*                      loop;                 /* �ܵ���������kloop_t */
    kaddress_t*                   peer_address;         /* �Զ˵�ַ */
    kaddress_t*                   local_address;        /* ���ص�ַ */
    knet_channel_event_e          event;                /* �ܵ�Ͷ���¼� */
    volatile knet_channel_state_e state;                /* �ܵ�״̬ */
    atomic_counter_t              ref_count;            /* ���ü��� */
    knet_channel_ref_cb_t         cb;                   /* �ص� */
    time_t                        last_recv_ts;         /* ���һ�ζ�����ʱ������룩 */
    time_t                        timeout;              /* �����г�ʱ���룩 */
    time_t                        last_connect_timeout; /* ���һ��connect()��ʱ���룩 */
    time_t                        connect_timeout;      /* connect()��ʱ������룩 */
    int                           auto_reconnect;       /* �Զ�������־ */
    int                           flag;                 /* ѡȡ����ʹ���Զ����־λ */
    void*                         data;                 /* ѡȡ����ʹ���Զ������� */
    void*                         user_data;            /* �û�����ָ�� - �ڲ�ʹ�� */
    void*                         user_ptr;             /* ��¶���ⲿʹ�õ�����ָ�� - �ⲿʹ�� */
    /* ��չ���ݳ�Ա */
    /*
     * ����ktimer_stop���رն�ʱ��, ��ʱ�����ڶ�ʱ��ѭ���ڱ�����, �ܵ���������ʱ��
     */
    ktimer_t*    recv_timeout_timer;    /* �����г�ʱ��ʱ�� */
    ktimer_t*    connect_timeout_timer; /* ���ӳ�ʱ��ʱ�� */
    volatile int close_cb_called;       /* �ر��¼��Ƿ��Ѿ������� */
} channel_ref_info_t;

/**
 * �ܵ�����
 */
struct _channel_ref_t {
    int                 share;     /* �Ƿ�ͨ��knet_channel_ref_share()���� */
    uint64_t            domain_id; /* ��ID */
    kdlist_node_t*      list_node; /* ������ڵ� */
    channel_ref_info_t* ref_info;  /* �ܵ���Ϣ */
};

/**
 * �ܵ���ʱ���ص�
 * @param timer �ܵ���ʱ��
 * @param data �ܵ�ָ��
 */
void timer_cb(ktimer_t* timer, void* data);

kchannel_ref_t* knet_channel_ref_create(kloop_t* loop, kchannel_t* channel) {
    kchannel_ref_t* channel_ref = create(kchannel_ref_t);
    verify(channel_ref);
    memset(channel_ref, 0, sizeof(kchannel_ref_t));
    channel_ref->ref_info = create(channel_ref_info_t);
    verify(channel_ref->ref_info);
    memset(channel_ref->ref_info, 0, sizeof(channel_ref_info_t));
    channel_ref->ref_info->stream = stream_create(channel_ref);
    verify(channel_ref->ref_info->stream);
    channel_ref->ref_info->channel      = channel;
    channel_ref->ref_info->ref_count    = 0;
    channel_ref->ref_info->loop         = loop;
    channel_ref->ref_info->last_recv_ts = time(0);
    channel_ref->ref_info->state        = channel_state_init;
    /* ��¼ͳ������ */
    knet_loop_profile_increase_active_channel_count(knet_loop_get_profile(loop));
    return channel_ref;
}

int knet_channel_ref_destroy(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    if (channel_ref->ref_info) {        
        if (channel_ref->ref_info->state == channel_state_init) {
            /* δ�����뵽������ */
            knet_channel_close(channel_ref->ref_info->channel);
        }
        /* ������ü��� */
        if (!atomic_counter_zero(&channel_ref->ref_info->ref_count)) {
            return error_ref_nonzero;
        }
        /* ���ٶԶ˵�ַ */
        if (channel_ref->ref_info->peer_address) {
            knet_address_destroy(channel_ref->ref_info->peer_address);
        }
        /* ���ٱ��ص�ַ */
        if (channel_ref->ref_info->local_address) {
            knet_address_destroy(channel_ref->ref_info->local_address);
        }
        /* ֪ͨѡȡ��ɾ���ܵ������Դ */
        if ((channel_ref->ref_info->state != channel_state_init) && /* �Ѿ������뵽loop�ܵ����� */
            channel_ref->ref_info->loop) {
            knet_impl_remove_channel_ref(channel_ref->ref_info->loop, channel_ref);
        }
        /* ���ٹܵ� */
        if (channel_ref->ref_info->channel) {
            knet_channel_destroy(channel_ref->ref_info->channel);
        }
        /* ���������� */
        if (channel_ref->ref_info->stream) {
            stream_destroy(channel_ref->ref_info->stream);
        }
        /* ���ٶ�ʱ�� */
        knet_channel_ref_stop_connect_timeout_timer(channel_ref);
        knet_channel_ref_stop_recv_timeout_timer(channel_ref);
        /* ���ٹܵ���Ϣ */
        knet_free(channel_ref->ref_info);
    }
    /* ���ٹܵ����� */
    knet_free(channel_ref);
    return error_ok;
}

int knet_channel_ref_connect(kchannel_ref_t* channel_ref, const char* ip, int port, int timeout) {
    int      error = error_ok;
    kloop_t* loop  = 0;
    verify(channel_ref);
    verify(port);
    if (!ip) {
        if (knet_channel_is_ipv6(channel_ref->ref_info->channel)) {
            ip = ":::";
        } else {
            ip = "0.0.0.0";
        }
    }
    if (knet_channel_ref_check_state(channel_ref, channel_state_connect)) {
        /* �Ѿ���������״̬ */
        return error_connect_in_progress;
    }
    if (!channel_ref->ref_info->peer_address) {
        /* �����Զ˵�ַ���� */
        if (knet_channel_is_ipv6(channel_ref->ref_info->channel)) {
            channel_ref->ref_info->peer_address = knet_address_create6();
        } else {
            channel_ref->ref_info->peer_address = knet_address_create();
        }
    }
    /* ���öԶ˵�ַ */
    knet_address_set(channel_ref->ref_info->peer_address, ip, port);
    if (timeout > 0) {
        channel_ref->ref_info->connect_timeout = timeout;
        /* ���ó�ʱʱ��� */
        channel_ref->ref_info->last_connect_timeout = time(0) + timeout;
    }
    /* ���Ŀ������ܾ�������ʧ�� */
    error = knet_channel_connect(channel_ref->ref_info->channel, ip, port);
    if (error_ok != error) {
        return error;
    }
    log_verb("start connecting to IP[%s], port[%d]", ip, port);
    /* ���ؾ��� */
    loop = knet_channel_ref_choose_loop(channel_ref);
    if (loop) {
        /* ����ԭloop��active�ܵ����� */
        knet_loop_profile_decrease_active_channel_count(
            knet_loop_get_profile(channel_ref->ref_info->loop));
        /* ����Ŀ��loop */
        channel_ref->ref_info->loop = loop;
        /* ����Ŀ��loop��active�ܵ����� */
        knet_loop_profile_increase_active_channel_count(knet_loop_get_profile(loop));
        /* ��ӵ�����loop */
        knet_loop_notify_connect(loop, channel_ref);
        return error_ok;
    }
    /* ��ǰ�߳��ڷ������� */
    return knet_channel_ref_connect_in_loop(channel_ref);
}

int knet_channel_ref_reconnect(kchannel_ref_t* channel_ref, int timeout) {
    int                   error               = error_ok; /* ������ */
    char                  ip[32]              = {0};      /* IP */
    int                   port                = 0;        /* �˿� */
    kchannel_ref_t*       new_channel         = 0;        /* ����ʱ�½����Ĺܵ� */
    kaddress_t*           peer_address        = 0;        /* �Զ˵�ַ */
    time_t                connect_timeout     = 0;        /* ���ӳ�ʱ(��) */
    knet_channel_ref_cb_t cb                  = 0;        /* �ܵ��ص� */
    kloop_t*              loop                = 0;        /* loop */
    uint32_t              max_send_list_len   = 0;        /* ����������󳤶� */
    uint32_t              max_recv_buffer_len = 0;        /* ���ջ�������󳤶� */
    int                   auto_reconnect      = 0;        /* �Զ�������־ */
    void*                 user_data           = 0;        /* �ڲ�ʹ������ָ�� */
    void*                 ptr                 = 0;        /* �û�����ָ�� */
    verify(channel_ref);
    verify(channel_ref->ref_info);
    verify(channel_ref->ref_info->channel);
    if (!knet_channel_ref_check_state(channel_ref, channel_state_connect)) {
        /* δ������������״̬�Ĺܵ��������� */
        return error_channel_not_connect;
    }
    /* ��ȡԭ�йܵ����� */
    loop                = knet_channel_ref_get_loop(channel_ref);
    max_send_list_len   = knet_channel_get_max_send_list_len(channel_ref->ref_info->channel);
    max_recv_buffer_len = knet_channel_get_max_recv_buffer_len(channel_ref->ref_info->channel);
    cb                  = knet_channel_ref_get_cb(channel_ref);
    user_data           = knet_channel_ref_get_user_data(channel_ref);
    ptr                 = knet_channel_ref_get_ptr(channel_ref);
    auto_reconnect      = knet_channel_ref_check_auto_reconnect(channel_ref);
    peer_address        = channel_ref->ref_info->peer_address;
    verify(peer_address);
    strcpy(ip, address_get_ip(peer_address));
    port = address_get_port(peer_address);
    /* �����¹ܵ� */
    if (knet_channel_is_ipv6(channel_ref->ref_info->channel)) {
        new_channel = knet_loop_create_channel6(loop, max_send_list_len, max_recv_buffer_len);
    } else {
        new_channel = knet_loop_create_channel(loop, max_send_list_len, max_recv_buffer_len);
    }
    verify(new_channel);
    if (timeout > 0) {
        /* �����µĳ�ʱʱ��� */
        connect_timeout = timeout;
    } else {
        /* ʹ��ԭ�еĳ�ʱʱ��� */
        if (channel_ref->ref_info->connect_timeout) {
            connect_timeout = channel_ref->ref_info->connect_timeout;
        }
    }
    /* ����ԭ�лص� */
    knet_channel_ref_set_cb(new_channel, cb);
    /* ����ԭ���û����� */
    knet_channel_ref_set_user_data(new_channel, user_data);
    /* �����û�ָ�� */
    knet_channel_ref_set_ptr(new_channel, ptr);
    /* �����Զ�������־ */
    knet_channel_ref_set_auto_reconnect(new_channel, auto_reconnect);
    /* �������ӳ�ʱ��ʱ�� */
    knet_channel_ref_stop_connect_timeout_timer(channel_ref);
    /* �����µ������� */
    error = knet_channel_ref_connect(new_channel, ip, port, (int)connect_timeout);
    if (error_ok != error) {
        return error;
    }
    /* ����ԭ�йܵ� */
    knet_channel_ref_close(channel_ref);
    return error;
}

void knet_channel_ref_set_auto_reconnect(kchannel_ref_t* channel_ref, int auto_reconnect) {
    verify(channel_ref);
    channel_ref->ref_info->auto_reconnect = auto_reconnect;
}

int knet_channel_ref_check_auto_reconnect(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->auto_reconnect;
}

void knet_channel_ref_accept_async(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    /* ��ӵ���Ծ�ܵ����� */
    knet_loop_add_channel_ref(channel_ref->ref_info->loop, channel_ref);
    /* ���ùܵ�״̬ */
    knet_channel_ref_set_state(channel_ref, channel_state_accept);
    /* ���ù�ע�¼� */
    knet_channel_ref_set_event(channel_ref, channel_event_recv);
}

int knet_channel_ref_accept(kchannel_ref_t* channel_ref, const char* ip, int port, int backlog) {
    int         error     = 0; /* ������ */
    thread_id_t thread_id = 0; /* loop���ڵ��߳�ID */
    verify(channel_ref);
    verify(port);
    if (knet_channel_ref_check_state(channel_ref, channel_state_accept)) {
        /* �Ѿ����ڼ���״̬ */
        return error_accept_in_progress;
    }
    /* ���� */
    error = knet_channel_accept(channel_ref->ref_info->channel, ip, port, backlog);
    if (error == error_ok) {
        thread_id = knet_loop_get_thread_id(channel_ref->ref_info->loop);
        if (thread_id) { /* kloop_t��ĳ���߳����й� */
            if (thread_id != thread_get_self_id()) { /* ���߳����������� */
                knet_loop_notify_accept_async(channel_ref->ref_info->loop, channel_ref);
                return error;
            }
        }
        /* ��ǰ�߳��� */
        knet_loop_add_channel_ref(channel_ref->ref_info->loop, channel_ref);
        /* ����Ϊ����״̬ */
        knet_channel_ref_set_state(channel_ref, channel_state_accept);
        /* Ͷ�ݶ��¼� */
        knet_channel_ref_set_event(channel_ref, channel_event_recv);
    }
    return error;
}

kchannel_ref_t* knet_channel_ref_share(kchannel_ref_t* channel_ref) {
    kchannel_ref_t* channel_ref_shared = 0;
    verify(channel_ref);
    channel_ref_shared = create(kchannel_ref_t);
    verify(channel_ref_shared);
    memset(channel_ref_shared, 0, sizeof(kchannel_ref_t));
    /* ���ӹܵ����ü��� */
    atomic_counter_inc(&channel_ref->ref_info->ref_count);
    /* ����ܵ���Ϣָ�� */
    channel_ref_shared->ref_info = channel_ref->ref_info;
    channel_ref_shared->share = 1;
    return channel_ref_shared;
}

void knet_channel_ref_leave(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    /* �ݼ����ü��� */
    atomic_counter_dec(&channel_ref->ref_info->ref_count);
    /* �ܵ���Ϣ������kloop_t���� */
    knet_free(channel_ref);
}

void knet_channel_ref_update_close_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    if (knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        /* �Ѿ����ӳٹر������� */
        return;
    }
    /* ����Ϊ�ر�״̬ */
    knet_channel_ref_set_state(channel_ref, channel_state_close);
    /* ȡ��Ͷ�ݶ���д�¼� */
    knet_channel_ref_clear_event(channel_ref, channel_event_recv | channel_event_send);
    /* �رչܵ� */
    knet_channel_close(channel_ref->ref_info->channel);
    /* �رչܵ����� */
    knet_loop_close_channel_ref(channel_ref->ref_info->loop, channel_ref);
    /* ���ٽ��ճ�ʱ��ʱ�� */
    knet_channel_ref_stop_recv_timeout_timer(channel_ref);
    /* �������ӳ�ʱ��ʱ�� */
    knet_channel_ref_stop_connect_timeout_timer(channel_ref);
}

void knet_channel_ref_close_check_reconnect(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    if (knet_channel_ref_check_auto_reconnect(channel_ref)) {
        /* �Զ����� */
        /* α�쵱ǰ״̬ */
        knet_channel_ref_set_state(channel_ref, channel_state_connect);
        /* ���� */
        knet_channel_ref_reconnect(channel_ref, 0);
    } else {
        /* �رչܵ� */
        knet_channel_ref_close(channel_ref);
    }
}

void knet_channel_ref_close(kchannel_ref_t* channel_ref) {
    kloop_t* loop = 0;
    verify(channel_ref);
    loop = channel_ref->ref_info->loop;
    if (!knet_loop_get_thread_id(loop) || (channel_ref->ref_info->state == channel_state_init)) {
        /* δ�����뵽������ */
        knet_channel_ref_destroy(channel_ref);
        return;
    }
    if (knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        /* �Ѿ��ڹر������� */
        return;
    }
    if (knet_loop_get_thread_id(loop) != thread_get_self_id()) {
        /* ֪ͨ�ܵ������߳� */
        log_info("close channel cross thread, notify thread[id:%ld]", knet_loop_get_thread_id(loop));
        knet_loop_notify_close(loop, channel_ref);
    } else {
        /* ���߳��ڹر� */
        log_info("close channel[%llu] in loop thread[id: %ld]", knet_channel_ref_get_uuid(channel_ref), knet_loop_get_thread_id(loop));
        knet_channel_ref_update_close_in_loop(loop, channel_ref);
    }
}

int knet_channel_ref_check_close(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return knet_channel_ref_check_state(channel_ref, channel_state_close);
}

void knet_channel_ref_update_send_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref, kbuffer_t* send_buffer) {
    int error = 0;
    verify(loop);
    verify(channel_ref);
    verify(send_buffer);
    /* ��¼ͳ������ */
    knet_loop_profile_add_send_bytes(knet_loop_get_profile(loop), knet_buffer_get_length(send_buffer));
    /* ������ */
    error = knet_channel_send_buffer(channel_ref->ref_info->channel, send_buffer);
    switch (error) {
    case error_send_patial: /* ���ַ��ͳɹ� */
        /* ����Ͷ��д�¼� */
        knet_channel_ref_set_event(channel_ref, channel_event_send);
        break;
    case error_send_fail: /* ����ʧ�� */
        knet_channel_ref_close_check_reconnect(channel_ref);
        break;
    default:
        break;
    }
}

int knet_channel_ref_write(kchannel_ref_t* channel_ref, const char* data, int size) {
    kloop_t*   loop        = 0;
    kbuffer_t* send_buffer = 0;
    int        error       = error_ok;
    verify(channel_ref);
    verify(data);
    verify(size);
    if (!knet_channel_ref_check_state(channel_ref, channel_state_active)) {
        return error_not_connected;
    }
    loop = channel_ref->ref_info->loop;
    if (knet_loop_get_thread_id(loop) != thread_get_self_id()) {
        /* ת��loop�����̷߳��� */
        log_info("send cross thread, notify thread[id:%ld]", knet_loop_get_thread_id(loop));
        send_buffer = knet_buffer_create(size);
        verify(send_buffer);
        if (!send_buffer) {
            return error_no_memory;
        }
        /* ����������� */
        knet_buffer_put(send_buffer, data, size);
        /* ֪ͨĿ���߳� */
        knet_loop_notify_send(loop, channel_ref, send_buffer);
    } else {
        knet_loop_profile_add_send_bytes(knet_loop_get_profile(channel_ref->ref_info->loop), size);
        /* ��ǰ�̷߳��� */
        error = knet_channel_send(channel_ref->ref_info->channel, data, size);
        switch (error) {
        case error_send_patial:
            knet_channel_ref_set_event(channel_ref, channel_event_send);
            /* ���ڵ����߲��Ǵ��� */
            error = error_ok;
            break;
        case error_send_fail: /* ����ʧ�� */
            knet_channel_ref_close_check_reconnect(channel_ref);
            break;
        default:
            break;
        }
    }
    return error;
}

socket_t knet_channel_ref_get_socket_fd(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return knet_channel_get_socket_fd(channel_ref->ref_info->channel);
}

kstream_t* knet_channel_ref_get_stream(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->stream;
}

kloop_t* knet_channel_ref_get_loop(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->loop;
}

void knet_channel_ref_set_loop_node(kchannel_ref_t* channel_ref, kdlist_node_t* node) {
    verify(channel_ref); /* node����Ϊ0 */
    channel_ref->ref_info->loop_node = node;
}

kdlist_node_t* knet_channel_ref_get_loop_node(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->loop_node;
}

void knet_channel_ref_set_event(kchannel_ref_t* channel_ref, knet_channel_event_e e) {
    verify(channel_ref);
    knet_impl_event_add(channel_ref, e);
    channel_ref->ref_info->event |= e;
}

knet_channel_event_e knet_channel_ref_get_event(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->event;
}

void knet_channel_ref_clear_event(kchannel_ref_t* channel_ref, knet_channel_event_e e) {
    verify(channel_ref);
    knet_impl_event_remove(channel_ref, e);
    channel_ref->ref_info->event &= ~e;
}

void knet_channel_ref_set_state(kchannel_ref_t* channel_ref, knet_channel_state_e state) {
    verify(channel_ref);
    channel_ref->ref_info->state = state;
}

int knet_channel_ref_check_state(kchannel_ref_t* channel_ref, knet_channel_state_e state) {
    verify(channel_ref);
    return (channel_ref->ref_info->state == state);
}

int knet_channel_ref_check_event(kchannel_ref_t* channel_ref, knet_channel_event_e event) {
    verify(channel_ref);
    return (channel_ref->ref_info->event & event);
}

kchannel_ref_t* knet_channel_ref_accept_from_socket_fd(kchannel_ref_t* channel_ref, kloop_t* loop, socket_t client_fd, int event, int ipv6) {
    kchannel_t*     acceptor_channel    = 0; /* �����ܵ� */
    uint32_t        max_send_list_len   = 0; /* ����������� */
    uint32_t        max_ringbuffer_size = 0; /* �����ܻ��������� */
    kchannel_t*     client_channel      = 0; /* �ͻ��˹ܵ� */
    kchannel_ref_t* client_ref          = 0; /* �ͻ��˹ܵ����� */
    verify(channel_ref);
    verify(channel_ref->ref_info);
    verify(client_fd > 0);
    acceptor_channel = channel_ref->ref_info->channel;
    verify(acceptor_channel);
    max_send_list_len = knet_channel_get_max_send_list_len(acceptor_channel);
    if (!max_send_list_len) {
        max_send_list_len = INT_MAX;
    }
    max_ringbuffer_size = ringbuffer_get_max_size(knet_channel_get_ringbuffer(acceptor_channel));
    if (!max_ringbuffer_size) {
        max_ringbuffer_size = 16 * 1024; /* Ĭ��16K */
    }
    /* �����ͻ��˹ܵ� */
    client_channel = knet_channel_create_exist_socket_fd(client_fd, max_send_list_len, max_ringbuffer_size, ipv6);
    verify(client_channel);
    /* �����ܵ����� */
    client_ref = knet_channel_ref_create(loop, client_channel);
    verify(client_ref);
    if (event) {
        /* ��ӵ���ǰ�߳�loop */
        knet_loop_add_channel_ref(channel_ref->ref_info->loop, client_ref);
        /* ������ͬʱ�����¼���״̬ */
        knet_channel_ref_set_state(client_ref, channel_state_active);
        knet_channel_ref_set_event(client_ref, channel_event_recv);
    }
    return client_ref;
}

void knet_channel_ref_update_accept(kchannel_ref_t* channel_ref) {
    kchannel_ref_t* client_ref = 0;
    kloop_t*        loop       = 0;
    socket_t        client_fd  = 0;
    int             ipv6 = knet_channel_is_ipv6(channel_ref->ref_info->channel);
    verify(channel_ref);
    /* �鿴ѡȡ���Ƿ����Զ���ʵ�� */
    client_fd = knet_impl_channel_accept(channel_ref);
    if (!client_fd) {
        /* Ĭ��ʵ�� */
        if (knet_channel_is_ipv6(channel_ref->ref_info->channel)) {
            client_fd = socket_accept6(knet_channel_get_socket_fd(channel_ref->ref_info->channel));
        } else {
            client_fd = socket_accept(knet_channel_get_socket_fd(channel_ref->ref_info->channel));
        }
    }
    if (client_fd <= 0) {
        return;
    }
    knet_channel_ref_set_state(channel_ref, channel_state_accept);
    knet_channel_ref_set_event(channel_ref, channel_event_recv);
    if (client_fd) {
        loop = knet_channel_ref_choose_loop(channel_ref);
        if (loop) {
            client_ref = knet_channel_ref_accept_from_socket_fd(channel_ref, loop, client_fd, 0, ipv6);
            verify(client_ref);
            knet_channel_ref_set_user_data(client_ref, channel_ref->ref_info->user_data);
            knet_channel_ref_set_ptr(client_ref, channel_ref->ref_info->user_ptr);
            /* ���ûص� */
            knet_channel_ref_set_cb(client_ref, channel_ref->ref_info->cb);
            /* ���ö����г�ʱ */
            knet_channel_ref_set_timeout(client_ref, (int)channel_ref->ref_info->timeout);
            /* ��ӵ�����loop */
            knet_loop_notify_accept(loop, client_ref);
        } else {
            client_ref = knet_channel_ref_accept_from_socket_fd(channel_ref, channel_ref->ref_info->loop, client_fd, 1, ipv6);
            verify(client_ref);
            knet_channel_ref_set_user_data(client_ref, channel_ref->ref_info->user_data);
            knet_channel_ref_set_ptr(client_ref, channel_ref->ref_info->user_ptr);
            /* ���ûص� */
            knet_channel_ref_set_cb(client_ref, channel_ref->ref_info->cb);
            /* ���ö����г�ʱ */
            knet_channel_ref_set_timeout(client_ref, (int)channel_ref->ref_info->timeout);
            /* ���ûص� */
            if (channel_ref->ref_info->cb) {
                channel_ref->ref_info->cb(client_ref, channel_cb_event_accept);
            }
            /* �������ճ�ʱ��ʱ�� */
            knet_channel_ref_start_recv_timeout_timer(client_ref);
        }
    }
}

int knet_channel_ref_start_connect_timeout_timer(kchannel_ref_t* channel_ref) {
    int       error         = error_ok;
    ktimer_t* connect_timer = 0;
    verify(channel_ref);
    /* ���� */
    knet_channel_ref_stop_connect_timeout_timer(channel_ref);
    /* �����µ� */
    if (channel_ref->ref_info->connect_timeout) {
        connect_timer = ktimer_create(knet_loop_get_timer_loop(channel_ref->ref_info->loop));
        verify(connect_timer);
        error = ktimer_start(connect_timer, knet_channel_ref_get_timer_cb(channel_ref),
            channel_ref, channel_ref->ref_info->connect_timeout * 1000);
        if (error == error_ok) {
            channel_ref->ref_info->connect_timeout_timer = connect_timer;
        }
    }
    return error;
}

int knet_channel_ref_start_recv_timeout_timer(kchannel_ref_t* channel_ref) {
    int       error      = error_ok;
    ktimer_t* recv_timer = 0;
    verify(channel_ref);
    /* ���� */
    knet_channel_ref_stop_recv_timeout_timer(channel_ref);
    /* �����µ� */
    if (channel_ref->ref_info->timeout) {
        /* ������ʱ�� */
        recv_timer = ktimer_create(knet_loop_get_timer_loop(channel_ref->ref_info->loop));
        verify(recv_timer);
        /* ������ʱ�� */
        error = ktimer_start(recv_timer, knet_channel_ref_get_timer_cb(channel_ref),
            channel_ref, channel_ref->ref_info->timeout * 1000);
        if (error == error_ok) {
            channel_ref->ref_info->recv_timeout_timer = recv_timer;
        }
    }
    return error;
}

void knet_channel_ref_stop_recv_timeout_timer(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    if (channel_ref->ref_info->recv_timeout_timer) {
        /* ֹͣ��ʱ�� */
        ktimer_stop(channel_ref->ref_info->recv_timeout_timer);
        /* ���� */
        channel_ref->ref_info->recv_timeout_timer = 0;
    }
}

void knet_channel_ref_update_accept_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref) {
    verify(loop);
    verify(channel_ref);
    /* ��ӵ���ǰ�߳�loop */
    knet_loop_add_channel_ref(loop, channel_ref);
    /* ���ùܵ�Ϊ��Ծ״̬ */
    knet_channel_ref_set_state(channel_ref, channel_state_active);
    /* Ͷ�ݶ��¼� */
    knet_channel_ref_set_event(channel_ref, channel_event_recv);    
    if (channel_ref->ref_info->cb) {
        /* ���ûص� */
        channel_ref->ref_info->cb(channel_ref, channel_cb_event_accept);
    }
    /* �������ճ�ʱ��ʱ�� */
    knet_channel_ref_start_recv_timeout_timer(channel_ref);
}

void knet_channel_ref_stop_connect_timeout_timer(kchannel_ref_t* channel_ref) {
    if (channel_ref->ref_info->connect_timeout_timer) {
        /* ֹͣ��ʱ�� */
        ktimer_stop(channel_ref->ref_info->connect_timeout_timer);
        /* ���� */
        channel_ref->ref_info->connect_timeout_timer = 0;
    }
}

void knet_channel_ref_update_connect(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    /* Ͷ�ݶ��¼� */
    knet_channel_ref_set_event(channel_ref, channel_event_recv);
    /* �л��ܵ�Ϊ��Ծ״̬ */
    knet_channel_ref_set_state(channel_ref, channel_state_active);
    if (channel_ref->ref_info->cb) {
        /* ���ûص� */
        log_error("channel connectd, channel[%llu]", knet_channel_ref_get_uuid(channel_ref));
        channel_ref->ref_info->cb(channel_ref, channel_cb_event_connect);
    }
    /* �������ӳ�ʱ��ʱ�� */
    knet_channel_ref_stop_connect_timeout_timer(channel_ref);
    /* �������ճ�ʱ��ʱ�� */
    knet_channel_ref_start_recv_timeout_timer(channel_ref);
}

void timer_cb(ktimer_t* timer, void* data) {
    time_t          now           = time(0); /* ��ǰʱ��(��) */
    kchannel_ref_t* channel_ref   = (kchannel_ref_t*)data; /* ��ǰ�ܵ� */
    time_t          gap           = now - channel_ref->ref_info->last_recv_ts; /* �ϴν��վ��뵱ǰʱ��(��) */
    ktimer_t*       recv_timer    = knet_channel_ref_get_recv_timeout_timer(channel_ref); /* ���ն�ʱ�� */
    ktimer_t*       connect_timer = knet_channel_ref_get_connect_timeout_timer(channel_ref); /* ���Ӷ�ʱ�� */
    if (connect_timer == timer) { /* ���ӳ�ʱ��ʱ�� */
        if (socket_check_send_ready(knet_channel_ref_get_socket_fd(channel_ref))) {
            knet_impl_event_add(channel_ref, channel_event_send);
        }
        /* ��δʵ�ʳ�ʱ */
        if (channel_ref->ref_info->last_connect_timeout > now) {
            return;
        }
        /* ���ӳ�ʱ */
        if (knet_channel_ref_get_cb(channel_ref)) {
            log_error("connect timeout, channel[%llu]", knet_channel_ref_get_uuid(channel_ref));
            knet_channel_ref_get_cb(channel_ref)(channel_ref, channel_cb_event_connect_timeout);
        }
        /* �Զ����� */
        if (knet_channel_ref_check_auto_reconnect(channel_ref)) {
            knet_channel_ref_reconnect(channel_ref, 0);
        }
    } else if (recv_timer == timer) { /* ���ճ�ʱ��ʱ�� */
        if (!knet_channel_ref_check_state(channel_ref, channel_state_accept)) {
            if (gap > channel_ref->ref_info->timeout) {
                /* ����ʱ������ */
                if (knet_channel_ref_get_cb(channel_ref)) {
                    knet_channel_ref_get_cb(channel_ref)(channel_ref, channel_cb_event_timeout);
                }
            }
        }
    }
}

ktimer_cb_t knet_channel_ref_get_timer_cb(kchannel_ref_t* channel_ref) {
    (void)channel_ref;
    return timer_cb;
}

void knet_channel_ref_update_recv(kchannel_ref_t* channel_ref) {
    int      error = 0;
    uint32_t bytes = 0;
    verify(channel_ref);
    /* ��ȡ�ܵ������ֽ����� */
    bytes = knet_stream_available(channel_ref->ref_info->stream);
    /* ������¼� */
    error = knet_channel_update_recv(channel_ref->ref_info->channel);
    if (error != error_ok) {
        bytes = knet_stream_available(channel_ref->ref_info->stream);
        if (bytes) {
            /* ��¼ͳ������ */
            knet_loop_profile_add_recv_bytes(knet_loop_get_profile(channel_ref->ref_info->loop),
                knet_stream_available(channel_ref->ref_info->stream) - bytes);
            if (channel_ref->ref_info->cb) {
                /* ���ûص� */
                channel_ref->ref_info->cb(channel_ref, channel_cb_event_recv);
            }
        }
    }
    switch (error) {
        case error_recv_fail: /* ����ʧ�� */
            knet_channel_ref_close_check_reconnect(channel_ref);
            break;
        case error_recv_buffer_full: /* ���ջ������� */
            knet_channel_ref_close_check_reconnect(channel_ref);
            break;
        default:
            break;
    }
    if (error == error_ok) {
        /* ��¼ͳ������ */
        knet_loop_profile_add_recv_bytes(knet_loop_get_profile(channel_ref->ref_info->loop),
            knet_stream_available(channel_ref->ref_info->stream) - bytes);
        if (channel_ref->ref_info->cb) {
            /* ���ûص� */
            channel_ref->ref_info->cb(channel_ref, channel_cb_event_recv);
        }
        /* ����Ͷ�ݶ��¼� */
        knet_channel_ref_set_event(channel_ref, channel_event_recv);
    }
}

void knet_channel_ref_update_send(kchannel_ref_t* channel_ref) {
    int error = 0;
    verify(channel_ref);
    /* �������¼� */
    error = knet_channel_update_send(channel_ref->ref_info->channel);
    switch (error) {
        case error_send_fail: /* ����ʧ�� */
            knet_channel_ref_close_check_reconnect(channel_ref);
            break;
        case error_send_patial: /* δ����ȫ������ */
            knet_channel_ref_set_event(channel_ref, channel_event_send);
            break;
        default:
            break;
    }
    if (error == error_ok) {
        if (channel_ref->ref_info->cb) {
            /* ���ûص� */
            channel_ref->ref_info->cb(channel_ref, channel_cb_event_send);
        }
    }
}

void knet_channel_ref_update(kchannel_ref_t* channel_ref, knet_channel_event_e e, time_t ts) {
    verify(channel_ref);
    if (knet_channel_ref_check_state(channel_ref, channel_state_close)) {
        /* �ܵ��Ѿ��ر� */
        return;
    }
    if ((e & channel_event_recv) && knet_channel_ref_check_event(channel_ref, channel_event_recv)) {
        if (knet_channel_ref_check_state(channel_ref, channel_state_accept)) {
            /* ������ */
            knet_channel_ref_update_accept(channel_ref);
        } else {
            /* ���һ�ζ�ȡ�����ݵ�ʱ������룩 */
            channel_ref->ref_info->last_recv_ts = ts;
            /* �� */
            knet_channel_ref_update_recv(channel_ref);
        }
    } 
    if ((e & channel_event_send) && knet_channel_ref_check_event(channel_ref, channel_event_send)) {
        if (knet_channel_ref_check_state(channel_ref, channel_state_connect)) {
            /* ������� */
            knet_channel_ref_update_connect(channel_ref);
        } else {
            /* д */
            knet_channel_ref_update_send(channel_ref);
        }
    }
}

kringbuffer_t* knet_channel_ref_get_ringbuffer(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return knet_channel_get_ringbuffer(channel_ref->ref_info->channel);
}

kloop_t* knet_channel_ref_choose_loop(kchannel_ref_t* channel_ref) {
    kloop_t*          loop         = 0;
    kloop_t*          current_loop = 0;
    kloop_balancer_t* balancer     = 0;
    verify(channel_ref);
    current_loop = channel_ref->ref_info->loop;
    balancer = knet_loop_get_balancer(current_loop);
    if (!balancer) {
        return 0;
    }
    /* ����Ƿ�����loop_balancer_out���� */
    if (knet_loop_check_balance_options(channel_ref->ref_info->loop, loop_balancer_out)) {
        loop = knet_loop_balancer_choose(balancer);
        if (loop == channel_ref->ref_info->loop) {
            return 0;
        }
    }
    return loop;
}

void knet_channel_ref_set_flag(kchannel_ref_t* channel_ref, int flag) {
    verify(channel_ref);
    channel_ref->ref_info->flag = flag;
}

int knet_channel_ref_get_flag(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->flag;
}

void knet_channel_ref_set_data(kchannel_ref_t* channel_ref, void* data) {
    verify(channel_ref);
    channel_ref->ref_info->data = data;
}

void* knet_channel_ref_get_data(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->data;
}

void knet_channel_ref_set_loop(kchannel_ref_t* channel_ref, kloop_t* loop) {
    verify(channel_ref);
    channel_ref->ref_info->loop = loop;
}

int knet_channel_ref_check_balance(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->balance;
}

void knet_channel_ref_set_timeout(kchannel_ref_t* channel_ref, int timeout) {
    verify(channel_ref); /* timeout����Ϊ0 */
    channel_ref->ref_info->timeout = (time_t)timeout;
}

int knet_channel_ref_get_timeout(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return (int)channel_ref->ref_info->timeout;
}

int knet_channel_ref_get_connect_timeout(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return (int)channel_ref->ref_info->connect_timeout;
}

void knet_channel_ref_set_cb(kchannel_ref_t* channel_ref, knet_channel_ref_cb_t cb) {
    verify(channel_ref);
    channel_ref->ref_info->cb = cb;
}

knet_channel_ref_cb_t knet_channel_ref_get_cb(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->cb;
}

int knet_channel_ref_connect_in_loop(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    /* ��ӵ���Ծ�ܵ����� */
    knet_loop_add_channel_ref(channel_ref->ref_info->loop, channel_ref);
    /* ��������״̬ */
    knet_channel_ref_set_state(channel_ref, channel_state_connect);
    /* ֪ͨѡȡ��Ͷ�ݷ����¼� */
    knet_channel_ref_set_event(channel_ref, channel_event_send);
    /* �������ӳ�ʱ��ʱ�� */
    return knet_channel_ref_start_connect_timeout_timer(channel_ref);;
}

kaddress_t* knet_channel_ref_get_peer_address(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    if (channel_ref->ref_info->peer_address) {
        return channel_ref->ref_info->peer_address;
    }
    /* ��һ�ν��� */
    channel_ref->ref_info->peer_address = knet_address_create();
    if (knet_channel_is_ipv6(channel_ref->ref_info->channel)) {
        socket_getpeername6(channel_ref, channel_ref->ref_info->peer_address);
    } else {
        socket_getpeername(channel_ref, channel_ref->ref_info->peer_address);
    }
    return channel_ref->ref_info->peer_address;
}

kaddress_t* knet_channel_ref_get_local_address(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    if (channel_ref->ref_info->local_address) {
        return channel_ref->ref_info->local_address;
    }
    /* ��һ�ν��� */
    channel_ref->ref_info->local_address = knet_address_create();
    if (channel_ref->ref_info->state != channel_state_init) {
        if (knet_channel_is_ipv6(channel_ref->ref_info->channel)) {
            socket_getsockname6(channel_ref, channel_ref->ref_info->local_address);
        } else {
            socket_getsockname(channel_ref, channel_ref->ref_info->local_address);
        }
    }
    return channel_ref->ref_info->local_address;
}

void knet_channel_ref_set_domain_node(kchannel_ref_t* channel_ref, kdlist_node_t* node) {
    verify(channel_ref);
    channel_ref->list_node = node;
}

kdlist_node_t* knet_channel_ref_get_domain_node(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->list_node;
}

int knet_channel_ref_check_share(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->share;
}

void knet_channel_ref_set_domain_id(kchannel_ref_t* channel_ref, uint64_t domain_id) {
    verify(channel_ref);
    channel_ref->domain_id = domain_id;
}

uint64_t knet_channel_ref_get_domain_id(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->domain_id;
}

int knet_channel_ref_incref(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return (int)atomic_counter_inc(&channel_ref->ref_info->ref_count);
}

int knet_channel_ref_decref(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return (int)atomic_counter_dec(&channel_ref->ref_info->ref_count);
}

int knet_channel_ref_is_ipv6(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return knet_channel_is_ipv6(channel_ref->ref_info->channel);
}

int knet_channel_ref_check_ref_zero(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return !atomic_counter_zero(&channel_ref->ref_info->ref_count);
}

int knet_channel_ref_get_ref(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->ref_count;
}

uint64_t knet_channel_ref_get_uuid(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return knet_channel_get_uuid(channel_ref->ref_info->channel);
}

int knet_channel_ref_equal(kchannel_ref_t* a, kchannel_ref_t* b) {
    return ((knet_channel_ref_get_uuid(a) == knet_channel_ref_get_uuid(b)) &&
        (a->ref_info == b->ref_info));
}

void knet_channel_ref_set_user_data(kchannel_ref_t* channel_ref, void* data) {
    verify(channel_ref);
    channel_ref->ref_info->user_data = data;
}

void* knet_channel_ref_get_user_data(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->user_data;
}

void knet_channel_ref_set_ptr(kchannel_ref_t* channel_ref, void* ptr) {
    verify(channel_ref);
    channel_ref->ref_info->user_ptr = ptr;
}

void* knet_channel_ref_get_ptr(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->user_ptr;
}

void knet_channel_ref_set_recv_timeout_timer(kchannel_ref_t* channel_ref, ktimer_t* timer) {
    verify(channel_ref);
    channel_ref->ref_info->recv_timeout_timer = timer;
}

ktimer_t* knet_channel_ref_get_recv_timeout_timer(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->recv_timeout_timer;
}

void knet_channel_ref_set_connect_timeout_timer(kchannel_ref_t* channel_ref, ktimer_t* timer) {
    verify(channel_ref);
    channel_ref->ref_info->connect_timeout_timer = timer;
}

ktimer_t* knet_channel_ref_get_connect_timeout_timer(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->connect_timeout_timer;
}

int knet_channel_ref_check_close_cb_called(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    return channel_ref->ref_info->close_cb_called;
}

void knet_channel_ref_set_close_cb_called(kchannel_ref_t* channel_ref) {
    verify(channel_ref);
    channel_ref->ref_info->close_cb_called = 1;
}
