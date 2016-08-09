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
 * thread_runner_start_loop可以直接以kloop_t作为参数在线程内运行knet_loop_run_once
 * thread_runner_start_timer_loop可以直接以ktimer_loop_t作为参数在线程内运行ktimer_loop_run_once
 * thread_runner_start_multi_loop_varg可以同时运行多个knet_loop_run_once或者ktimer_loop_run_once
 * </pre>
 * @{
 */

/**
 * 创建一个线程
 * @param func 线程函数
 * @param params 参数
 * @return kthread_runner_t实例
 */
extern kthread_runner_t* thread_runner_create(knet_thread_func_t func, void* params);

/**
 * 销毁一个线程
 * @param runner kthread_runner_t实例
 */
extern void thread_runner_destroy(kthread_runner_t* runner);

/**
 * 启动线程
 * @param runner kthread_runner_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start(kthread_runner_t* runner, int stack_size);

/**
 * 停止线程
 * @param runner kthread_runner_t实例
 */
extern void thread_runner_stop(kthread_runner_t* runner);

/**
 * 获取线程ID
 * @param runner kthread_runner_t实例
 * @return 线程ID
 */
extern thread_id_t thread_runner_get_id(kthread_runner_t* runner);

/**
 * 在线程内运行knet_loop_run()
 * @param runner kthread_runner_t实例
 * @param loop kloop_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_loop(kthread_runner_t* runner, kloop_t* loop, int stack_size);

/**
 * 在线程内运行timer_loop_run()
 * @param runner kthread_runner_t实例
 * @param timer_loop ktimer_loop_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_timer_loop(kthread_runner_t* runner, ktimer_loop_t* timer_loop, int stack_size);

/**
 * 在线程内启动多个kloop_t或ktimer_loop_t
 *
 * format内可以有多个kloop_t（l）或者ktimer_loop_t（t），譬如：lt，标识一个kloop_t，一个ktimer_loop_t
 * @param runner kthread_runner_t实例
 * @param stack_size 栈大小
 * @param format 启动字符串
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_multi_loop_varg(kthread_runner_t* runner, int stack_size, const char* format, ...);

/**
 * 等待线程终止
 * @param runner kthread_runner_t实例
 */
extern void thread_runner_join(kthread_runner_t* runner);

/**
 * 终止线程
 * @param runner kthread_runner_t实例
 */
extern void thread_runner_exit(kthread_runner_t* runner);

/**
 * 检查线程是否正在运行
 * @param runner kthread_runner_t实例
 * @retval 0 未运行
 * @retval 非零 正在运行
 */
extern int thread_runner_check_start(kthread_runner_t* runner);

/**
 * 取得线程运行参数，thread_runner_create()第二个参数
 * @param runner kthread_runner_t实例
 * @return 线程运行参数
 */
extern void* thread_runner_get_params(kthread_runner_t* runner);

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
 * @param runner kthread_runner_t实例
 * @param data 自定义数据指针
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int thread_set_tls_data(kthread_runner_t* runner, void* data);

/**
 * 取得线程本地存储
 * @param runner kthread_runner_t实例
 * @retval 0 获取失败或不存在
 * @retval 有效指针
 */
void* thread_get_tls_data(kthread_runner_t* runner);

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
 * 原子操作 - CAS(check and swap)
 * @param counter atomic_counter_t实例
 * @param target 目标值
 * @param value 新值
 * @return 操作后的值
 */
extern atomic_counter_t atomic_counter_cas(atomic_counter_t* counter,
    atomic_counter_t target, atomic_counter_t value);

/**
 * 原子操作 - 赋值
 * @param counter atomic_counter_t实例
 * @param value 新值
 * @return 操作后的值
 */
extern atomic_counter_t atomic_counter_set(atomic_counter_t* counter,
    atomic_counter_t value);

/**
 * 原子操作 - 是否为零
 * @param counter atomic_counter_t实例
 * @retval 0 非零
 * @retval 其他 零
 */
extern int atomic_counter_zero(atomic_counter_t* counter);

/**
 * 建立互斥锁实例
 * @return klock_t实例
 */
extern klock_t* lock_create();

/**
 * 销毁互斥锁
 * @param lock klock_t实例
 */
extern void lock_destroy(klock_t* lock);

/**
 * 锁
 * @param lock klock_t实例
 */
extern void lock_lock(klock_t* lock);

/**
 * 测试锁
 * @param lock klock_t实例
 * @sa pthread_mutex_trylock
 */
extern int lock_trylock(klock_t* lock);

/**
 * 解锁
 * @param lock klock_t实例
 */
extern void lock_unlock(klock_t* lock);

/**
 * 建立读写锁
 * @return krwlock_t实例
 */
extern krwlock_t* rwlock_create();

/**
 * 销毁读写锁
 * @param rwlock krwlock_t实例
 */
extern void rwlock_destroy(krwlock_t* rwlock);

/**
 * 读者锁
 * @param rwlock krwlock_t实例
 */
extern void rwlock_rdlock(krwlock_t* rwlock);

/**
 * 读者解锁
 * @param rwlock krwlock_t实例
 */
extern void rwlock_rdunlock(krwlock_t* rwlock);

/**
 * 写者锁
 * @param rwlock krwlock_t实例
 */
extern void rwlock_wrlock(krwlock_t* rwlock);

/**
 * 写者解锁
 * @param rwlock krwlock_t实例
 */
extern void rwlock_wrunlock(krwlock_t* rwlock);

/**
 * 建立条件变量
 * @return kcond_t实例
 */
extern kcond_t* cond_create();

/**
 * 销毁条件变量
 * @param cond kcond_t实例
 */
extern void cond_destroy(kcond_t* cond);

/**
 * 等待唤醒
 * @param cond kcond_t实例
 * @param lock 锁
 */
extern void cond_wait(kcond_t* cond, klock_t* lock);

/**
 * 等待唤醒
 * @param cond kcond_t实例
 * @param lock 锁
 * @param ms 等待时间（毫秒）
 */
extern void cond_wait_ms(kcond_t* cond, klock_t* lock, int ms);

/**
 * 唤醒
 * @param cond kcond_t实例
 */
extern void cond_signal(kcond_t* cond);

/** @} */

#endif /* THREAD_API_H */
