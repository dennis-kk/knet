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

#include "channel.h"
#include "ringbuffer.h"
#include "loop.h"
#include "misc.h"
#include "logger.h"

/**
 * �ܵ�
 */
struct _channel_t {
    kringbuffer_t* send_ringbuffer;   /* ���ͻ��λ�����, ͨ��socket����ʧ�ܵ����ݻ���������������, �ȴ��´η���*/
    kringbuffer_t* recv_ringbuffer;   /* �����λ�����, ͨ��socket��ȡ�������������ݻ��������������� */
    socket_t      socket_fd;         /* �׽��� */
    uint64_t      uuid;             /* �ܵ�UUID */
    int          ipv6;             /* �Ƿ���IPV6 */
};

kchannel_t* knet_channel_create(uint32_t max_send_list_len, uint32_t recv_ring_len, int ipv6) {
    socket_t socket_fd = 0;
    /* ����socket������ */
    if (ipv6) {
        socket_fd = socket_create6();
    } else {
        socket_fd = socket_create();
    }
    verify(socket_fd > 0);
    if (socket_fd <= 0) {
        return 0;
    }
    /* �����ܵ� */
    return knet_channel_create_exist_socket_fd(socket_fd, max_send_list_len, recv_ring_len, ipv6);
}

kchannel_t* knet_channel_create_exist_socket_fd(socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len, int ipv6) {
    kchannel_t* channel = knet_create(kchannel_t);
    verify(channel);
    (void)max_send_list_len;
    memset(channel, 0, sizeof(kchannel_t));
    channel->uuid = uuid_create(); /* �ܵ�UUID */
    channel->send_ringbuffer = ringbuffer_create(recv_ring_len); /* д������ */
    verify(channel->send_ringbuffer);
    channel->recv_ringbuffer = ringbuffer_create(recv_ring_len); /* �������� */
    verify(channel->recv_ringbuffer);
    channel->socket_fd      = socket_fd;
    channel->ipv6          = ipv6;
    /* ����Ϊ������ */
    socket_set_non_blocking_on(channel->socket_fd);
    /* �ر��ӳٷ��� */
    socket_set_nagle_off(channel->socket_fd);
    /* �ر�TIME_WAIT */
    socket_set_linger_off(channel->socket_fd);
    /* �ر�keep alive */
    socket_set_keepalive_off(channel->socket_fd);
    /* ���������/д���������� */
    socket_set_recv_buffer_size(channel->socket_fd, recv_ring_len);
    socket_set_send_buffer_size(channel->socket_fd, recv_ring_len);
    return channel;
}

void knet_channel_destroy(kchannel_t* channel) {
    verify(channel);
    /* ���ٷ��ͻ����� */
    if (channel->send_ringbuffer) {
        ringbuffer_destroy(channel->send_ringbuffer);
    }
    /* ���ٽ��ջ����� */
    if (channel->recv_ringbuffer) {
        ringbuffer_destroy(channel->recv_ringbuffer);
    }
    /* �ر�socket */
    knet_channel_close(channel);
    /* ���ٹܵ� */
    knet_free(channel);
}

int knet_channel_connect(kchannel_t* channel, const char* ip, int port) {
    verify(channel);
    verify(ip);
    /* �������Ӳ��� */
    if (channel->ipv6) {
        return socket_connect6(channel->socket_fd, ip, port);
    } else {
        return socket_connect(channel->socket_fd, ip, port);
    }
}

int knet_channel_accept(kchannel_t* channel, const char* ip, int port, int backlog) {
    verify(channel);
    verify(port);
    if (!backlog) {
        backlog = 500;
    }
    if (!ip) {
        if (channel->ipv6) {
            ip = "0:0:0:0:0:0:0:0";
        } else {
            ip = "0.0.0.0";
        }
    }
    /* ����Ϊ����״̬ */
    if (channel->ipv6) {
        return socket_bind_and_listen6(channel->socket_fd, ip, port, backlog);
    } else {
        return socket_bind_and_listen(channel->socket_fd, ip, port, backlog);
    }
}

int knet_channel_send_buffer(kchannel_t* channel) {
    int bytes = 0;
    int size = 0;
    char* ptr = 0;
    verify(channel);
    verify(channel->send_ringbuffer);
    if (knet_channel_send_buffer_reach_max(channel)) {
        /* ʼ���޷����� */
        return error_send_fail;
    }
    for (; (size = ringbuffer_read_lock_size(channel->send_ringbuffer));) {
        ptr = ringbuffer_read_lock_ptr(channel->send_ringbuffer);
        bytes = socket_send(channel->socket_fd, ptr, size);
        if (bytes <= 0) {
            /* ���󣬹ر� */
            ringbuffer_read_commit(channel->send_ringbuffer, 0);
            return error_send_fail;
        } else {
            /* ���ͳɹ� */
            ringbuffer_read_commit(channel->send_ringbuffer, (uint32_t)bytes);
        }
    }
    return (ringbuffer_empty(channel->send_ringbuffer) ? error_ok : error_send_patial);
}

int knet_channel_send(kchannel_t* channel, const char* data, int size) {
    int        bytes       = 0;
    /*kbuffer_t* send_buffer = 0;*/
    verify(channel);
    verify(data);
    verify(size);
    verify(channel->send_ringbuffer);
    /* ʼ���޷����� */
    if (knet_channel_send_buffer_reach_max(channel)) {
        return error_send_fail;
    }
    if (ringbuffer_empty(channel->send_ringbuffer)) {
        /* ����ֱ�ӷ��� */
        bytes = socket_send(channel->socket_fd, data, size);
    }
    if (bytes < 0) {
        return error_send_fail;
    }
    /* ֱ�ӷ���ʧ�ܣ�����û�з�����ϵ��ֽڷ��뷢������ȴ��´η��� */
    if (size > bytes) {
      if ((size - bytes) != (int)ringbuffer_write(channel->send_ringbuffer, data + bytes, size - bytes)) {
          return error_send_fail;
      }
      /* ��Ҫ�Ժ��� */
      return error_send_patial;
    }
    return error_ok;
}

int knet_channel_update_send(kchannel_t* channel) {
    verify(channel);
    verify(channel->send_ringbuffer);
    return knet_channel_send_buffer(channel);
}

int knet_channel_update_recv(kchannel_t* channel) {
    int      bytes      = 0; /* ����socket_recvʵ�ʽ��յ��ֽ� */
    int      recv_bytes = 0; /* ���յ��ֽ����� */
    uint32_t size       = 0; /* ����������ǰ������д����ֽ��� */ 
    char*    ptr        = 0; /* ��������������д�����ʼ��ַ */
    verify(channel);
    verify(channel->recv_ringbuffer);
    if (ringbuffer_full(channel->recv_ringbuffer)) {
        /* �������������ر�, ������, �ɸ������������С */
        return error_recv_buffer_full;
    }
    for (; (size = ringbuffer_write_lock_size(channel->recv_ringbuffer));) {
        ptr   = ringbuffer_write_lock_ptr(channel->recv_ringbuffer);
        bytes = socket_recv(channel->socket_fd, ptr, size);
        if (bytes < 0) {
            /* ���󣬹ر� */
            ringbuffer_write_commit(channel->recv_ringbuffer, 0);
            return error_recv_fail;
        } else if (bytes == 0) {
            /* δ���յ�, �´μ������� */
            ringbuffer_write_commit(channel->recv_ringbuffer, 0);
            return error_ok;
        } else {
            recv_bytes += bytes;
            /* ���յ� */
            ringbuffer_write_commit(channel->recv_ringbuffer, (uint32_t)bytes);
        }
    }
    if (!recv_bytes) {
        /* ���β�����ɽ��գ��ǹر������ */
        return error_recv_nothing;
    }
    return error_ok;
}

void knet_channel_close(kchannel_t* channel) {
    verify(channel);
    if (0 == channel->socket_fd) {
        return;
    }
    socket_close(channel->socket_fd);
    channel->socket_fd = 0;
}

socket_t knet_channel_get_socket_fd(kchannel_t* channel) {
    verify(channel);
    return channel->socket_fd;
}

kringbuffer_t* knet_channel_get_ringbuffer(kchannel_t* channel) {
    verify(channel);
    return channel->recv_ringbuffer;
}

uint32_t knet_channel_get_max_send_list_len(kchannel_t* channel) {
  (void)channel;
  return 0;
}

uint32_t knet_channel_get_max_recv_buffer_len(kchannel_t* channel) {
    verify(channel);
    return ringbuffer_get_max_size(channel->recv_ringbuffer);
}

uint64_t knet_channel_get_uuid(kchannel_t* channel) {
    verify(channel);
    return channel->uuid;
}

int knet_channel_send_buffer_reach_max(kchannel_t* channel) {
    verify(channel);
    return ringbuffer_full(channel->send_ringbuffer);
}

int knet_channel_is_ipv6(kchannel_t* channel) {
    verify(channel);
    return channel->ipv6;
}
