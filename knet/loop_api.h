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

#ifndef LOOP_API_H
#define LOOP_API_H

#include "config.h"

/*
 * 创建一个事件循环
 * @return loop_t实例
 */
extern loop_t* loop_create();

/*
 * 销毁事件循环
 * 事件循环内的所有管道也会被销毁
 * @param loop loop_t实例
 */
extern void loop_destroy(loop_t* loop);

/*
 * 创建管道
 * @param loop loop_t实例
 * @param max_send_list_len 发送缓冲区链最大长度
 * @param recv_ring_len 接受环形缓冲区最大长度
 * @return channel_ref_t实例
 */
extern channel_ref_t* loop_create_channel(loop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len);

/*
 * 使用已存在的套接字创建管道
 * @param loop loop_t实例
 * @param socket_fd 套接字
 * @param max_send_list_len 发送缓冲区链最大长度
 * @param recv_ring_len 接受环形缓冲区最大长度
 * @return channel_ref_t实例
 */
extern channel_ref_t* loop_create_channel_exist_socket_fd(loop_t* loop, socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len);

/*
 * 运行一次事件循环
 * loop_t不是线程安全的，不能在多个线程内同时对同一个loop_t实例调用loop_run_once
 * @param loop loop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int loop_run_once(loop_t* loop);

/*
 * 运行事件循环直到调用loop_exit()
 * loop_t不是线程安全的，不能在多个线程内同时对同一个loop_t实例调用loop_run
 * @param loop loop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int loop_run(loop_t* loop);

/*
 * 退出函数loop_run()
 * @param loop loop_t实例
 */
extern void loop_exit(loop_t* loop);

/*
 * 获取活跃管道数量
 * @param loop loop_t实例
 * @return 活跃管道数量
 */
extern int loop_get_active_channel_count(loop_t* loop);

/*
 * 获取已关闭管道数量
 * @param loop loop_t实例
 * @return 关闭管道数量
 */
extern int loop_get_close_channel_count(loop_t* loop);

#endif /* LOOP_API_H */
