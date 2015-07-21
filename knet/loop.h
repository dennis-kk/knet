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

#ifndef LOOP_H
#define LOOP_H

#include "config.h"
#include "loop_api.h"

/**
 * 添加kchannel_ref_t实例到活跃链表
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_add_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 从到活跃链表删除kchannel_ref_t实例
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_remove_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 从到活跃链表删除kchannel_ref_t实例，并放入关闭链表
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_close_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 取得活跃链表
 * @param loop kloop_t实例
 * @return kdlist_t实例
 */
kdlist_t* knet_loop_get_active_list(kloop_t* loop);

/**
 * 取得关闭链表
 * @param loop kloop_t实例
 * @return kdlist_t实例
 */
kdlist_t* knet_loop_get_close_list(kloop_t* loop);

/**
 * 设置选取器实现
 * @param loop kloop_t实例
 * @param impl 选取器实现
 */
void knet_loop_set_impl(kloop_t* loop, void* impl);

/**
 * 取得选取器实现
 * @param loop kloop_t实例
 * @return 选取器实现
 */
void* knet_loop_get_impl(kloop_t* loop);

/**
 * 取得选取器当前线程ID
 * @param loop kloop_t实例
 * @return 线程ID
 */
thread_id_t knet_loop_get_thread_id(kloop_t* loop);

/**
 * 设置负载均衡器(kloop_balancer_t实例）
 * @param loop kloop_t实例
 * @param balancer kloop_balancer_t实例
 */
void knet_loop_set_balancer(kloop_t* loop, kloop_balancer_t* balancer);

/**
 * 取得负载均衡器(kloop_balancer_t实例）
 * @param loop kloop_t实例
 * @return kloop_balancer_t实例
 */
kloop_balancer_t* knet_loop_get_balancer(kloop_t* loop);

/**
 * 发送事件通知 - 监听到新连接
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_notify_accept(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 发起监听通知 - 当前loop内监听
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_notify_accept_async(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 发送事件通知 - 发起连接
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_notify_connect(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 发送事件通知 - 跨线程发送
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 * @param send_buffer kbuffer_t实例
 */
void knet_loop_notify_send(kloop_t* loop, kchannel_ref_t* channel_ref, kbuffer_t* send_buffer);

/**
 * 发送事件通知 - 关闭管道
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_loop_notify_close(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 通知管道回调函数
 * @param channel kchannel_ref_t实例
 * @param e 管道事件
 */
void knet_loop_queue_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e);

/**
 * 触发读事件回调knet_loop_queue_cb
 * @param loop kloop_t实例
 */
void knet_loop_notify(kloop_t* loop);

/**
 * 处理事件
 * @param loop kloop_t实例
 */
void knet_loop_event_process(kloop_t* loop);

/**
 * 检查活跃管道空闲超时
 * @param loop kloop_t实例
 * @param ts 当前时间戳（秒）
 */
void knet_loop_check_timeout(kloop_t* loop, time_t ts);

/**
 * 检查关闭管道是否可以销毁
 * @param loop kloop_t实例
 */
void knet_loop_check_close(kloop_t* loop);

/**
 * 检查是否正在运行
 * @param loop kloop_t实例
 */
int knet_loop_check_running(kloop_t* loop);

/**
 * 设置负载均衡配置
 * @param loop kloop_t实例
 * @param options 选项（loop_balancer_in， loop_balancer_out）
 */
void knet_loop_set_balance_options(kloop_t* loop, knet_loop_balance_option_e options);

/**
 * 取得负载均衡配置
 * @param loop kloop_t实例
 * @return 负载均衡配置
 */
knet_loop_balance_option_e knet_loop_get_balance_options(kloop_t* loop);

/**
 * 检查负载均衡配置是否开启
 * @param loop kloop_t实例
 * @param options 负载均衡配置
 * @retval 0 未开启
 * @retval 非零 开启
 */
int knet_loop_check_balance_options(kloop_t* loop, knet_loop_balance_option_e options);

/* 
 * 选取器需要实现的函数 - 建立并初始化
 * @param loop kloop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */

int knet_impl_create(kloop_t* loop);

/* 
 * 选取器需要实现的函数 - 停止并销毁
 * @param loop kloop_t实例
 */
void knet_impl_destroy(kloop_t* loop);

/* 
 * 选取器需要实现的函数 - 运行一次事件循环
 * @param loop kloop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_impl_run_once(kloop_t* loop);

/* 
 * 选取器需要实现的函数 - 投递一个（多个）事件
 * @param loop kloop_t实例
 * @param e 事件
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_impl_event_add(kchannel_ref_t* channel_ref, knet_channel_event_e e);

/* 
 * 选取器需要实现的函数 - 取消一个（多个）事件
 * @param loop kloop_t实例
 * @param e 事件
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_impl_event_remove(kchannel_ref_t* channel_ref, knet_channel_event_e e);

/* 
 * 选取器需要实现的函数 - 通知有新的管道加入了活跃链表
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_impl_add_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref);

/* 
 * 选取器需要实现的函数 - 通知管道关闭并销毁
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_impl_remove_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref);

/* 
 * 选取器需要实现的函数 - 新连接到来时检测选取器自定义实现
 * @param channel_ref kchannel_ref_t实例
 * @return 套接字
 */
socket_t knet_impl_channel_accept(kchannel_ref_t* channel_ref);

/**
 * 取得用户数据指针
 * @param loop kloop_t实例
 * @return 用户数据指针
 */
void* knet_loop_get_data(kloop_t* loop);

#endif /* LOOP_H */
