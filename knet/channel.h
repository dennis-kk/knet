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

#ifndef CHANNEL_H
#define CHANNEL_H

#include "config.h"

/**
 * 创建一个kchannel_t实例
 * @param max_send_list_len 发送链表最大长度
 * @param recv_ring_len 接受缓冲区最大长度
 * @param ipv6 是否是IPV6
 * @return kchannel_t实例
 */
kchannel_t* knet_channel_create(uint32_t max_send_list_len, uint32_t recv_ring_len, int ipv6);

/**
 * 创建一个kchannel_t实例
 * @param socket_fd 已建立的套接字
 * @param max_send_list_len 发送链表最大长度
 * @param recv_ring_len 接受缓冲区最大长度
 * @param ipv6 是否是IPV6
 * @return kchannel_t实例
 */
kchannel_t* knet_channel_create_exist_socket_fd(socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len, int ipv6);

/**
 * 销毁kchannel_t实例
 * @param channel kchannel_t实例
 */
void knet_channel_destroy(kchannel_t* channel);

/**
 * 连接监听器
 * @param channel kchannel_t实例
 * @param ip IP
 * @param port 端口
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_connect(kchannel_t* channel, const char* ip, int port);

/**
 * 监听
 * @param channel kchannel_t实例
 * @param ip IP
 * @param port 端口
 * @param backlog 等待队列长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_accept(kchannel_t* channel, const char* ip, int port, int backlog);

/**
 * 关闭
 * @param channel kchannel_t实例
 */
void knet_channel_close(kchannel_t* channel);

/**
 * 发送
 * 当发送链表为空的时候，会首先尝试直接发送到套接字缓冲区(zero copy)，否则会放到发送链表末尾等待
 * 适当时机发送.
 * @param channel kchannel_t实例
 * @param data 发送数据指针
 * @param size 数据长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_send(kchannel_t* channel, const char* data, int size);

/**
 * 发送
 * 放到发送链表末尾等待适当时机发送.
 * @param channel kchannel_t实例
 * @param send_buffer 发送缓冲区kbuffer_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_send_buffer(kchannel_t* channel);

/**
 * 可写事件通知
 * @param channel kchannel_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_update_send(kchannel_t* channel);

/**
 * 可读事件通知
 * @param channel kchannel_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_update_recv(kchannel_t* channel);

/**
 * 取得套接字
 * @param channel kchannel_t实例
 * @return 套接字
 */
socket_t knet_channel_get_socket_fd(kchannel_t* channel);

/**
 * 取得读缓冲区
 * @param channel kchannel_t实例
 * @return kringbuffer_t实例
 */
kringbuffer_t* knet_channel_get_ringbuffer(kchannel_t* channel);

/**
 * 取得发送链表最大长度限制
 * @param channel kchannel_t实例
 * @return 发送链表最大长度限制
 */
uint32_t knet_channel_get_max_send_list_len(kchannel_t* channel);

/**
 * 取得接收缓冲区最大长度限制
 * @param channel kchannel_t实例
 * @return 接收缓冲区最大长度限制
 */
uint32_t knet_channel_get_max_recv_buffer_len(kchannel_t* channel);

/**
 * 获取管道UUID
 * @param channel kchannel_t实例
 * @return 管道UUID
 */
uint64_t knet_channel_get_uuid(kchannel_t* channel);

/**
 * 发送链表内缓冲区数量是否达到最大
 * @param channel kchannel_t实例
 * @retval 0 不是
 * @retval 非零 是
 */
int knet_channel_send_buffer_reach_max(kchannel_t* channel);

/**
 * 是否是IPV6
 * @param channel kchannel_t实例
 * @retval 0 不是
 * @retval 非零 是
 */
int knet_channel_is_ipv6(kchannel_t* channel);

#endif /* CHANNEL_H */