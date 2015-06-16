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

#include "channel.h"
#include "buffer.h"
#include "list.h"
#include "ringbuffer.h"
#include "loop.h"
#include "misc.h"
#include "logger.h"

struct _channel_t {
    dlist_t*      send_buffer_list;  /* 发送链表 */
    uint32_t      max_send_list_len; /* 发送链表最大长度 */
    ringbuffer_t* recv_ringbuffer;   /* 读环形缓冲区 */
    socket_t      socket_fd;         /* 套接字 */
};

channel_t* channel_create(uint32_t max_send_list_len, uint32_t recv_ring_len) {
    socket_t socket_fd = socket_create();
    assert(socket_fd > 0);
    return channel_create_exist_socket_fd(socket_fd, max_send_list_len, recv_ring_len);
}

channel_t* channel_create_exist_socket_fd(socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len) {
    channel_t* channel = create(channel_t);
    assert(channel);
    channel->send_buffer_list = dlist_create();
    assert(channel->send_buffer_list);
    channel->recv_ringbuffer = ringbuffer_create(recv_ring_len);
    assert(channel->recv_ringbuffer);
    channel->max_send_list_len = max_send_list_len;
    channel->socket_fd = socket_fd;
    /* 设置为非阻塞 */
    socket_set_non_blocking_on(channel->socket_fd);
    /* 关闭延迟发送 */
    socket_set_nagle_off(channel->socket_fd);
    /* 关闭TIME_WAIT */
    socket_set_linger_off(channel->socket_fd);
    /* 关闭keep alive */
    socket_set_keepalive_off(channel->socket_fd);
    return channel;
}

void channel_destroy(channel_t* channel) {
    dlist_node_t* node        = 0;
    dlist_node_t* temp        = 0;
    buffer_t*     send_buffer = 0;
    assert(channel);
    /* 销毁未发送的数据 */
    dlist_for_each_safe(channel->send_buffer_list, node, temp) {
        send_buffer = (buffer_t*)dlist_node_get_data(node);
        buffer_destroy(send_buffer);
    }
    dlist_destroy(channel->send_buffer_list);
    /* 销毁接收缓冲区 */
    ringbuffer_destroy(channel->recv_ringbuffer);
    destroy(channel);
}

int channel_connect(channel_t* channel, const char* ip, int port) {
    assert(channel);
    assert(ip);
    return socket_connect(channel->socket_fd, ip, port);
}

int channel_accept(channel_t* channel, const char* ip, int port, int backlog) {
    assert(channel);
    assert(port);
    assert(backlog);
    if (!ip) {
        ip = "0.0.0.0";
    }
    /* 设置为监听状态 */
    return socket_bind_and_listen(channel->socket_fd, ip, port, backlog);
}

int channel_send_buffer(channel_t* channel, buffer_t* send_buffer) {
    assert(channel);
    assert(send_buffer);
    /* 将发送缓冲区加到链表尾部 */
    dlist_add_tail_node(channel->send_buffer_list, send_buffer);
    /* 让调用者重新设置写事件 */
    return error_send_patial;
}

int channel_send(channel_t* channel, const char* data, int size) {
    int       bytes       = 0;
    buffer_t* send_buffer = 0;
    assert(channel);
    assert(data);
    assert(size);
    if (dlist_empty(channel->send_buffer_list)) {
        /* 尝试直接发送 */
        bytes = socket_send(channel->socket_fd, data, size);
    }
    if (bytes < 0) {
        return error_send_fail;
    }
    /* 直接发送失败，或者没有发送完毕的字节放入发送链表等待下次发送 */
    if (size > bytes) {
        send_buffer = buffer_create(size - bytes);
        buffer_put(send_buffer, data + bytes, size - bytes);
        dlist_add_tail_node(channel->send_buffer_list, send_buffer);
        /* 需要稍后发送 */
        return error_send_patial;
    }
    return error_ok;
}

int channel_update_send(channel_t* channel) {
    dlist_node_t* node        = 0;
    dlist_node_t* temp        = 0;
    buffer_t*     send_buffer = 0;
    int           bytes       = 0;
    assert(channel);
    /* 发送链表内所有数据 */
    dlist_for_each_safe(channel->send_buffer_list, node, temp) {
        send_buffer = (buffer_t*)dlist_node_get_data(node);
        bytes = socket_send(channel->socket_fd, buffer_get_ptr(send_buffer), buffer_get_length(send_buffer));
        if (bytes < 0) {
            return error_send_fail;
        }
        if (buffer_get_length(send_buffer) > (uint32_t)bytes) {
            /* 本次未发送完毕，调整buffer长度，等待下次发送 */
            buffer_adjust(send_buffer, bytes);
            /* 部分发送 */
            return error_send_patial;
        } else {
            /* 销毁已发送节点 */
            buffer_destroy(send_buffer);
            dlist_delete(channel->send_buffer_list, node);
        }
    }
    /* 全部发送 */
    return error_ok;
}

int channel_update_recv(channel_t* channel) {
    int      bytes      = 0;
    int      recv_bytes = 0;
    uint32_t size       = 0;
    char*    ptr        = 0;
    assert(channel);
    if (ringbuffer_full(channel->recv_ringbuffer)) {
        /* 读缓冲区满，关闭, 防攻击, 可根据需求调整大小 */
        return error_recv_buffer_full;
    }
    for (; (size = ringbuffer_write_lock_size(channel->recv_ringbuffer));) {
        ptr = ringbuffer_write_lock_ptr(channel->recv_ringbuffer);
        bytes = socket_recv(channel->socket_fd, ptr, size);
        if (bytes < 0) {
            /* 错误，关闭 */
            return error_recv_fail;
        } else if (bytes == 0) {
            /* 未接收到, 下次继续接收 */
            ringbuffer_write_commit(channel->recv_ringbuffer, 0);
            return error_ok;
        } else {
            recv_bytes += bytes;
            /* 接收到 */
            ringbuffer_write_commit(channel->recv_ringbuffer, (uint32_t)bytes);
        }
    }
    if (!recv_bytes) {
        /* 本次不能完成接收，非关闭类错误 */
        return error_recv_nothing;
    }
    return error_ok;
}

void channel_close(channel_t* channel) {
    assert(channel);
    socket_close(channel->socket_fd);
}

socket_t channel_get_socket_fd(channel_t* channel) {
    assert(channel);
    return channel->socket_fd;
}

ringbuffer_t* channel_get_ringbuffer(channel_t* channel) {
    assert(channel);
    return channel->recv_ringbuffer;
}

uint32_t channel_get_max_send_list_len(channel_t* channel) {
    assert(channel);
    return channel->max_send_list_len;
}
