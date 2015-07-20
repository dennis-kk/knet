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
 * 3. 提供了统一的配置接口方便建立多个连接器或者监听器
 *
 * 默认情况下工作线程池内只有一个线程，可以通过配置接口函数framework_config_set_worker_thread_count
 * 进行配置. 框架默认会启动两个线程，一个监听线程，一个工作线程(工作线程池内唯一线程)，所有accept或者
 * connect的管道都会被分派到不同的工作线程处理，监听线程只负责连接的建立不负责读写.
 *
 * framework_config_t提供了统一的配置接口，在framework_t建立后可以获取framework_config_t
 * 接口进行配置，调用framework_start启动成功后将不能更改已经建立的配置，但可以在运行中增加新的监听器和连接器.
 *
 * 框架的回调函数将在线程池内被回调，如果有多个工作线程的情况下，应该保证回调函数内的代码是线程安全的，
 * 为了方便使用，框架的回调函数也会通知channel_cb_event_accept事件.
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
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_start(framework_t* f);

/**
 * 启动框架并等待框架关闭
 * @param f framework_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_start_wait(framework_t* f);

/**
 * 启动框架并等待框架关闭，如果启动失败或者框架关闭则销毁
 * @param f framework_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_start_wait_destroy(framework_t* f);

/**
 * 等待框架停止
 * @param f framework_t实例
 */
extern void framework_wait_for_stop(framework_t* f);

/**
 * 等待框架停止并销毁
 * @param f framework_t实例
 */
extern void framework_wait_for_stop_destroy(framework_t* f);

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
 * 启动监听器
 * @param f framework_t实例
 * @param c framework_acceptor_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_acceptor_start(framework_t* f, framework_acceptor_config_t* c);

/**
 * 启动连接器
 * @param f framework_t实例
 * @param c framework_connector_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int framework_connector_start(framework_t* f, framework_connector_config_t* c);

/** @} */

#endif /* FRAMEWORK_API_H */
