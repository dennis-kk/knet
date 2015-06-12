#ifndef THREAD_API_H
#define THREAD_API_H

#include "config.h"

/*
 * 创建一个线程
 * @param func 线程函数
 * @param params 参数
 * @return thread_runner_t实例
 */
extern thread_runner_t* thread_runner_create(thread_func_t func, void* params);

/*
 * 销毁一个线程
 * @param runner thread_runner_t实例
 */
extern void thread_runner_destroy(thread_runner_t* runner);

/*
 * 启动线程
 * @param runner thread_runner_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start(thread_runner_t* runner, int stack_size);

/*
 * 停止线程
 * @param runner thread_runner_t实例
 */
extern void thread_runner_stop(thread_runner_t* runner);

/*
 * 在线程内运行loop_run()
 * @param runner thread_runner_t实例
 * @param loop loop_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_loop(thread_runner_t* runner, loop_t* loop, int stack_size);

/*
 * 在线程内运行timer_loop_run()
 * @param runner thread_runner_t实例
 * @param timer_loop ktimer_loop_t实例
 * @param stack_size 线程栈大小（字节）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int thread_runner_start_timer_loop(thread_runner_t* runner, ktimer_loop_t* timer_loop, int stack_size);

/*
 * 等待线程终止
 * @param runner thread_runner_t实例
 */
extern void thread_runner_join(thread_runner_t* runner);

/*
 * 检查线程是否正在运行
 * @param runner thread_runner_t实例
 * @retval 0 未运行
 * @retval 非零 正在运行
 */
extern int thread_runner_check_start(thread_runner_t* runner);

/*
 * 取得线程运行参数，thread_runner_create()第二个参数
 * @param runner thread_runner_t实例
 * @return 线程运行参数
 */
extern void* thread_runner_get_params(thread_runner_t* runner);

/*
 * 取得线程ID
 * @param runner thread_runner_t实例
 * @return 线程ID
 */
extern thread_id_t thread_get_self_id();

/*
 * 睡眠
 * @param ms 睡眠时间（毫秒）
 */
extern void thread_sleep_ms(int ms);

/*
 * 原子操作 - 递增
 * @param counter atomic_counter_t实例
 * @return 递增后的值
 */
extern atomic_counter_t atomic_counter_inc(atomic_counter_t* counter);

/*
 * 原子操作 - 递减
 * @param counter atomic_counter_t实例
 * @return 递减后的值
 */
extern atomic_counter_t atomic_counter_dec(atomic_counter_t* counter);

/*
 * 原子操作 - 是否为零
 * @param counter atomic_counter_t实例
 * @retval 0 非零
 * @retval 其他 零
 */
extern int atomic_counter_zero(atomic_counter_t* counter);

#endif /* THREAD_API_H */
