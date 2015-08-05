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

#ifndef MISC_H
#define MISC_H

#include "config.h"
#include "thread_api.h"
#include "misc_api.h"

/**
 * 建立一个套接字
 * @return 套接字
 */
socket_t socket_create();

/**
 * 发起异步connect
 * @param socket_fd 套接字
 * @param ip IP
 * @param port 端口
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int socket_connect(socket_t socket_fd, const char* ip, int port);

/**
 * bind & listen
 * @param socket_fd 套接字
 * @param ip IP
 * @param port 端口
 * @param backlog 函数listen()参数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int socket_bind_and_listen(socket_t socket_fd, const char* ip, int port, int backlog);

/**
 * accept
 * @param socket_fd 套接字
 * @retval 0 失败
 * @retval 有效的套接字
 */
socket_t socket_accept(socket_t socket_fd);

/**
 * 关闭套接字（强制关闭）
 * @param socket_fd 套接字
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_close(socket_t socket_fd);

/**
 * 开启地址重用
 * @param socket_fd
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_reuse_addr_on(socket_t socket_fd);

/**
 * 开启套接字非阻塞
 * @param socket_fd
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_non_blocking_on(socket_t socket_fd);

/**
 * 关闭nagle算法
 *
 * 让数据更快的被发送
 * @param socket_fd
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_nagle_off(socket_t socket_fd);

/**
 * 关闭linger
 *
 * 避免已关闭的端口进入TIME_WAIT状态
 * @param socket_fd
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_linger_off(socket_t socket_fd);

/**
 * 关闭keep-alive
 * @param socket_fd
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_keepalive_off(socket_t socket_fd);

/**
 * 开启SO_DONTROUTE，未使用
 * @param socket_fd
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_donot_route_on(socket_t socket_fd);

/**
 * 设置接收缓冲区大小
 * @param socket_fd
 * @param size 接收缓冲区大小
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_recv_buffer_size(socket_t socket_fd, int size);

/**
 * 设置发送缓冲区大小
 * @param socket_fd
 * @param size 发送缓冲区大小
 * @retval 0 成功
 * @retval 其他 失败
 */
int socket_set_send_buffer_size(socket_t socket_fd, int size);

/**
 * 发送
 * @param socket_fd
 * @param data 数据指针
 * @param size 数据长度
 * @retval >0 成功
 * @retval 其他 失败
 */
int socket_send(socket_t socket_fd, const char* data, uint32_t size);

/**
 * 接收
 * @param socket_fd
 * @param data 接收缓冲区指针
 * @param size 接收缓冲长度
 * @retval >0 成功
 * @retval 其他 失败
 */
int socket_recv(socket_t socket_fd, char* data, uint32_t size);

/**
 * socketpair
 * @sa socketpair
 */
int socket_pair(socket_t pair[2]);

/**
 * getpeername
 * @sa getpeername
 */
int socket_getpeername(kchannel_ref_t* channel_ref, kaddress_t* address);

/**
 * getsockname
 * @sa getsockname
 */
int socket_getsockname(kchannel_ref_t* channel_ref, kaddress_t* address);

/**
 * 检查套接字是否可写
 * @retval 0 不可写
 * @retval 1 可写
 */
int socket_check_send_ready(socket_t socket_fd);

#endif /* MISC_H */
