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

#ifndef THREAD_API_H
#define THREAD_API_H

#include "config.h"

/**
 * @defgroup thread 线程
 * 线程相关
 *
 * <pre>
 * 线程
 *
 * 线程API提供了基本的线程相关操作:
 *
 * 1. 线程建立销毁
 * 2. TLS
 * 3. 原子操作
 *
 * thread_runner_start_loop可以直接以loop_t作为参数在线程内运行loop_run
 * thread_runner_start_timer_loop可以直接以ktimer_loop_t作为参数在线程内运行ktimer_loop_run
 *
 * </pre>
 * @{
 */

/**
 * 创建一个线程
 * @param func 线程函数
 * @param params 参数
 * @return thread_runner_t实例
 */
extern thread_runner_t* thread_runner_create(thread_func_t func, void* params);

/**
 * 销毁一个线程
 * @param runner thread_runner_t实例
 */
extern void thread_runner_destroy(thread_runner_t* runner);

/**
 * 启动线程
 * @param runner thread_runner_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start(thread_runner_t* runner, int stack_size);

/**
 * 停止线程
 * @param runner thread_runner_t实例
 */
extern void thread_runner_stop(thread_runner_t* runner);

/**
 * 在线程内运行loop_run()
 * @param runner thread_runner_t实例
 * @param loop loop_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_loop(thread_runner_t* runner, loop_t* loop, int stack_size);

/**
 * 在线程内运行timer_loop_run()
 * @param runner thread_runner_t实例
 * @param timer_loop ktimer_loop_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_timer_loop(thread_runner_t* runner, ktimer_loop_t* timer_loop, int stack_size);

/**
 * 等待线程终止
 * @param runner thread_runner_t实例
 */
extern void thread_runner_join(thread_runner_t* runner);

/**
 * 检查线程是否正在运行
 * @param runner thread_runner_t实例
 * @retval 0 未运行
 * @retval 非零 正在运行
 */
extern int thread_runner_check_start(thread_runner_t* runner);

/**
 * 取得线程运行参数，thread_runner_create()第二个参数
 * @param runner thread_runner_t实例
 * @return 线程运行参数
 */
extern void* thread_runner_get_params(thread_runner_t* runner);

/**
 * 取得线程ID
 * @return 线程ID
 */
extern thread_id_t thread_get_self_id();

/**
 * 睡眠
 * @param ms 睡眠时间（毫秒）
 */
extern void thread_sleep_ms(int ms);

/**
 * 设置线程本地存储
 * @param runner thread_runner_t实例
 * @param data 自定义数据指针
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int thread_set_tls_data(thread_runner_t* runner, void* data);

/**
 * 取得线程本地存储
 * @param runner thread_runner_t实例
 * @retval 0 获取失败或不存在
 * @retval 有效指针
 */
void* thread_get_tls_data(thread_runner_t* runner);

/**
 * 原子操作 - 递增
 * @param counter atomic_counter_t实例
 * @return 递增后的值
 */
extern atomic_counter_t atomic_counter_inc(atomic_counter_t* counter);

/**
 * 原子操作 - 递减
 * @param counter atomic_counter_t实例
 * @return 递减后的值
 */
extern atomic_counter_t atomic_counter_dec(atomic_counter_t* counter);

/**
 * 原子操作 - 是否为零
 * @param counter atomic_counter_t实例
 * @retval 0 非零
 * @retval 其他 零
 */
extern int atomic_counter_zero(atomic_counter_t* counter);

/** @} */

#endif /* THREAD_API_H */
