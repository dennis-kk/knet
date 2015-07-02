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

#ifndef MISC_H
#define MISC_H

#include "config.h"
#include "thread_api.h"

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

int socket_set_reuse_addr_on(socket_t socket_fd);
int socket_set_non_blocking_on(socket_t socket_fd);
int socket_set_nagle_off(socket_t socket_fd);
int socket_set_linger_off(socket_t socket_fd);
int socket_set_keepalive_off(socket_t socket_fd);
int socket_set_donot_route_on(socket_t socket_fd);
int socket_set_recv_buffer_size(socket_t socket_fd, int size);
int socket_set_send_buffer_size(socket_t socket_fd, int size);
int socket_send(socket_t socket_fd, const char* data, uint32_t size);
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
int socket_getpeername(channel_ref_t* channel_ref, address_t* address);

/**
 * getsockname
 * @sa getsockname
 */
int socket_getsockname(channel_ref_t* channel_ref, address_t* address);

/**
 * 检查套接字是否可写
 * @retval 0 不可写
 * @retval 1 可写
 */
int socket_check_send_ready(socket_t socket_fd);

/**
 * 建立互斥锁实例
 * @return lock_t实例
 */
lock_t* lock_create();

/**
 * 销毁互斥锁
 * @param lock lock_t实例
 */
void lock_destroy(lock_t* lock);

/**
 * 锁
 * @param lock lock_t实例
 */
void lock_lock(lock_t* lock);

/**
 * 测试锁
 * @param lock lock_t实例
 * @sa pthread_mutex_trylock
 */
int lock_trylock(lock_t* lock);

/**
 * 解锁
 * @param lock lock_t实例
 */
void lock_unlock(lock_t* lock);

/**
 * 获取当前毫秒
 */
uint32_t time_get_milliseconds();

/**
 * 获取当前微秒
 */
uint64_t time_get_microseconds();

/**
 * gettimeofday
 * @sa gettimeofday
 */
int time_gettimeofday(struct timeval *tp, void *tzp);

/**
 * 取得当前可阅读时间字符串
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 格式为YYYY-MM-DD HH:mm:SS:MS
 */
char* time_get_string(char* buffer, int size);

/**
 * 产生一个伪UUID，只保证本进程内不重复
 * @return 伪UUID
 */
uint64_t uuid_create();

/**
 * 取得当前工作目录
 * @param buffer 路径缓冲区指针
 * @param size 缓冲区大小
 * @retval 0 失败
 * @retval 路径缓冲区指针
 */
char* path_getcwd(char* buffer, int size);

/**
 * 获取最新的系统错误码
 * @return 系统错误码
 */
sys_error_t sys_get_errno();

/**
 * 字节序转换 - 主机序到网络序
 * @param ui64 64位无符号整数
 * @return 64位无符号整数
 */
uint64_t htonll(uint64_t ui64);

/**
 * 字节序转换 - 网络序到主机序
 * @param ui64 64位无符号整数
 * @return 64位无符号整数
 */
uint64_t ntohll(uint64_t ui64);

#endif /* MISC_H */
