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
 * 添加channel_ref_t实例到活跃链表
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_add_channel_ref(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 从到活跃链表删除channel_ref_t实例
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_remove_channel_ref(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 从到活跃链表删除channel_ref_t实例，并放入关闭链表
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_close_channel_ref(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 取得活跃链表
 * @param loop loop_t实例
 * @return dlist_t实例
 */
dlist_t* loop_get_active_list(loop_t* loop);

/**
 * 取得关闭链表
 * @param loop loop_t实例
 * @return dlist_t实例
 */
dlist_t* loop_get_close_list(loop_t* loop);

/**
 * 设置选取器实现
 * @param loop loop_t实例
 * @param impl 选取器实现
 */
void loop_set_impl(loop_t* loop, void* impl);

/**
 * 取得选取器实现
 * @param loop loop_t实例
 * @return 选取器实现
 */
void* loop_get_impl(loop_t* loop);

/**
 * 取得选取器当前线程ID
 * @param loop loop_t实例
 * @return 线程ID
 */
thread_id_t loop_get_thread_id(loop_t* loop);

/**
 * 设置负载均衡器(loop_balancer_t实例）
 * @param loop loop_t实例
 * @param balancer loop_balancer_t实例
 */
void loop_set_balancer(loop_t* loop, loop_balancer_t* balancer);

/**
 * 取得负载均衡器(loop_balancer_t实例）
 * @param loop loop_t实例
 * @return loop_balancer_t实例
 */
loop_balancer_t* loop_get_balancer(loop_t* loop);

/**
 * 发送事件通知 - 监听到新连接
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_notify_accept(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 发起监听通知 - 当前loop内监听
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_notify_accept_async(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 发送事件通知 - 发起连接
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_notify_connect(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 发送事件通知 - 跨线程发送
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 * @param send_buffer buffer_t实例
 */
void loop_notify_send(loop_t* loop, channel_ref_t* channel_ref, buffer_t* send_buffer);

/**
 * 发送事件通知 - 关闭管道
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 */
void loop_notify_close(loop_t* loop, channel_ref_t* channel_ref);

/**
 * 通知管道回调函数
 * @param channel channel_ref_t实例
 * @param e 管道事件
 */
void loop_queue_cb(channel_ref_t* channel, channel_cb_event_e e);

/**
 * 触发读事件回调loop_queue_cb
 * @param loop loop_t实例
 */
void loop_notify(loop_t* loop);

/**
 * 处理事件
 * @param loop loop_t实例
 */
void loop_event_process(loop_t* loop);

/**
 * 检查活跃管道空闲超时
 * @param loop loop_t实例
 * @param ts 当前时间戳（秒）
 */
void loop_check_timeout(loop_t* loop, time_t ts);

/**
 * 检查关闭管道是否可以销毁
 * @param loop loop_t实例
 */
void loop_check_close(loop_t* loop);

/**
 * 检查是否正在运行
 * @param loop loop_t实例
 */
int loop_check_running(loop_t* loop);

/**
 * 设置负载均衡配置
 * @param loop loop_t实例
 * @param options 选项（loop_balancer_in， loop_balancer_out）
 */
void loop_set_balance_options(loop_t* loop, loop_balance_option_e options);

/**
 * 取得负载均衡配置
 * @param loop loop_t实例
 * @return 负载均衡配置
 */
loop_balance_option_e loop_get_balance_options(loop_t* loop);

/**
 * 检查负载均衡配置是否开启
 * @param loop loop_t实例
 * @param options 负载均衡配置
 * @retval 0 未开启
 * @retval 非零 开启
 */
int loop_check_balance_options(loop_t* loop, loop_balance_option_e options);

/* 
 * 选取器需要实现的函数 - 建立并初始化
 * @param loop loop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */

int impl_create(loop_t* loop);

/* 
 * 选取器需要实现的函数 - 停止并销毁
 * @param loop loop_t实例
 */
void impl_destroy(loop_t* loop);

/* 
 * 选取器需要实现的函数 - 运行一次事件循环
 * @param loop loop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int impl_run_once(loop_t* loop);

/* 
 * 选取器需要实现的函数 - 投递一个（多个）事件
 * @param loop loop_t实例
 * @param e 事件
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int impl_event_add(channel_ref_t* channel_ref, channel_event_e e);

/* 
 * 选取器需要实现的函数 - 取消一个（多个）事件
 * @param loop loop_t实例
 * @param e 事件
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int impl_event_remove(channel_ref_t* channel_ref, channel_event_e e);

/* 
 * 选取器需要实现的函数 - 通知有新的管道加入了活跃链表
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int impl_add_channel_ref(loop_t* loop, channel_ref_t* channel_ref);

/* 
 * 选取器需要实现的函数 - 通知管道关闭并销毁
 * @param loop loop_t实例
 * @param channel_ref channel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int impl_remove_channel_ref(loop_t* loop, channel_ref_t* channel_ref);

/* 
 * 选取器需要实现的函数 - 新连接到来时检测选取器自定义实现
 * @param channel_ref channel_ref_t实例
 * @return 套接字
 */
socket_t impl_channel_accept(channel_ref_t* channel_ref);

/**
 * 取得用户数据指针
 * @param loop loop_t实例
 * @return 用户数据指针
 */
void* loop_get_data(loop_t* loop);

#endif /* LOOP_H */
