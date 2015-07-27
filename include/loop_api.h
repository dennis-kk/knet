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

#ifndef LOOP_API_H
#define LOOP_API_H

#include "config.h"

/**
 * @defgroup loop 事件循环
 * 网络事件循环
 *
 * <pre>
 * 网络事件API，作为各个不同操作系统网络选取器的包装，屏蔽了不同平台的具体实现，
 * 为你提供统一的调用接口.
 *
 * 管道引用kchannel_ref_t通过调用knet_loop_create_channel和knet_loop_create_channel_exist_socket_fd
 * 创建，knet_loop_run将启动事件循环并等待调用knet_loop_exit退出，你可以手动调用knet_loop_run_once运行一次事件
 * 循环自己控制循环的调用频率.
 *
 * 每个kloop_t内都维护了活跃管道和已关闭(未销毁)管道的双向链表，可以通过knet_loop_get_active_channel_count
 * 和knet_loop_get_close_channel_count来取得具体数量.
 *
 * 在创建管道时，要注意两个重要的配置参数：
 *
 * 1. max_send_list_len 发送链表最大元素个数
 * 2. recv_ring_len     接受缓冲区最大长度
 *
 * 通常向管道内发送数据(stream_push_系列)，管道会尝试直接发送，并不缓存要发送的数据，如果因为某种原因
 * 导致不能直接发送，数据会被缓存在发送链表内等待合适的时机发送，如果发送链表的长度达到上限，管道会被关闭.
 * 同样，接受缓冲区会从套接字内将数据读取出来，如果你一直不从kstream_t内取数据，那么早晚会被写满，管道也
 * 会被关闭.
 *
 * </pre>
 * @{
 */

/**
 * 创建一个事件循环
 * @return kloop_t实例
 */
extern kloop_t* knet_loop_create();

/**
 * 销毁事件循环
 * 事件循环内的所有管道也会被销毁
 * @param loop kloop_t实例
 */
extern void knet_loop_destroy(kloop_t* loop);

/**
 * 创建管道
 * @param loop kloop_t实例
 * @param max_send_list_len 发送缓冲区链最大长度
 * @param recv_ring_len 接受环形缓冲区最大长度
 * @return kchannel_ref_t实例
 */
extern kchannel_ref_t* knet_loop_create_channel(kloop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len);

/**
 * 使用已存在的套接字创建管道
 * @param loop kloop_t实例
 * @param socket_fd 套接字
 * @param max_send_list_len 发送缓冲区链最大长度
 * @param recv_ring_len 接受环形缓冲区最大长度
 * @return kchannel_ref_t实例
 */
extern kchannel_ref_t* knet_loop_create_channel_exist_socket_fd(kloop_t* loop, socket_t socket_fd,
    uint32_t max_send_list_len, uint32_t recv_ring_len);

/**
 * 运行一次事件循环
 * kloop_t不是线程安全的，不能在多个线程内同时对同一个kloop_t实例调用knet_loop_run_once
 * @param loop kloop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_loop_run_once(kloop_t* loop);

/**
 * 运行事件循环直到调用knet_loop_exit()
 * kloop_t不是线程安全的，不能在多个线程内同时对同一个kloop_t实例调用knet_loop_run
 * @param loop kloop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_loop_run(kloop_t* loop);

/**
 * 退出函数knet_loop_run()
 * @param loop kloop_t实例
 */
extern void knet_loop_exit(kloop_t* loop);

/**
 * 获取活跃管道数量
 * @param loop kloop_t实例
 * @return 活跃管道数量
 */
extern int knet_loop_get_active_channel_count(kloop_t* loop);

/**
 * 获取已关闭管道数量
 * @param loop kloop_t实例
 * @return 关闭管道数量
 */
extern int knet_loop_get_close_channel_count(kloop_t* loop);

/**
 * 设置用户数据指针
 * @param loop kloop_t实例
 * @param data 用户数据指针
 */
extern void knet_loop_set_data(kloop_t* loop, void* data);

/**
 * 取得统计器
 * @param loop kloop_t实例
 * @return kloop_profile_t实例
 */
extern kloop_profile_t* knet_loop_get_profile(kloop_t* loop);

/** @} */

#endif /* LOOP_API_H */
