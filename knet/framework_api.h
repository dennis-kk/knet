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

#ifndef FRAMEWORK_API_H
#define FRAMEWORK_API_H

#include "config.h"

/**
 * @defgroup framework 框架
 * 网络框架
 *
 * <pre>
 * 为方便使用提供了framework_t接口，对多线程模式(也包含单逻辑线程)提供了包装：
 *
 * 1. 独立的监听线程，处理channel_cb_event_accept事件，监听线程不处理管道读写
 * 2. 提供了一个工作线程池，使用loop_balancer_t(负载均衡器)来分派管道
 *
 * 默认情况下工作线程池内只有一个线程，可以通过配置接口函数framework_config_set_worker_thread_count
 * 进行配置. 框架默认会启动两个线程，一个监听线程，一个工作线程(工作线程池内唯一线程).
 *
 * framework_config_t提供了统一的配置接口，在framework_t建立后可以获取framework_config_t
 * 接口进行配置，调用framework_start启动成功后将不能更改配置.
 *
 * framework_start的第二个参数为管道回调函数，与channel_ref_set_cb所设置的回调函数相同，
 * 但框架的回调函数将在线程池内被回调，如果有多个工作线程的情况下，应该保证回调函数内的代码是线程安全的.
 *
 * </pre>
 * @{
 */

/**
 * 建立框架
 * @return framework_t实例
 */
extern framework_t* framework_create();

/**
 * 销毁框架
 * @param f framework_t实例
 */
extern void framework_destroy(framework_t* f);

/**
 * 启动框架
 * @param f framework_t实例
 * @param cb 框架回调函数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_start(framework_t* f, channel_ref_cb_t cb);

/**
 * 关闭框架
 * @param f framework_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_stop(framework_t* f);

/**
 * 取得框架配置
 * @param f framework_t实例
 * @return framework_config_t实例
 */
extern framework_config_t* framework_get_config(framework_t* f);

/**
 * 设置IP地址和监听端口
 * @param c framework_config_t实例
 * @param ip IP
 * @param port 端口
 */
extern void framework_config_set_address(framework_config_t* c, const char* ip, int port);

/**
 * 设置backlog
 * @param c framework_config_t实例
 * @param backlog 等待队列长度
 */
extern void framework_config_set_backlog(framework_config_t* c, int backlog);

/**
 * 设置工作线程数量，默认为单线程
 * @param c framework_config_t实例
 * @param worker_thread_count 工作线程数量
 */
extern void framework_config_set_worker_thread_count(framework_config_t* c, int worker_thread_count);

/**
 * 设置接受到连接的发送队列最大长度
 * @param c framework_config_t实例
 * @param max_send_list 接受到连接的发送队列最大长度
 */
extern void framework_config_set_max_send_list(framework_config_t* c, int max_send_list);

/**
 * 设置接受到连接的接收缓冲区最大长度
 * @param c framework_config_t实例
 * @param max_recv_buffer 接受到连接的接收缓冲区最大长度
 */
extern void framework_config_set_max_recv_buffer(framework_config_t* c, int max_recv_buffer);

/**
 * 设置接受到连接的心跳超时时间
 * @param c framework_config_t实例
 * @param max_idle_timeout 接受到连接的心跳超时时间
 */
extern void framework_config_set_max_idle_timeout(framework_config_t* c, int max_idle_timeout);

/** @} */

#endif /* FRAMEWORK_API_H */
