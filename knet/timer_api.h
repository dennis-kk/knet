#ifndef TIMER_API_H
#define TIMER_API_H

#include "config.h"

/*
 * WARNING 未经测试，需要等测试完成
 */

/*
 * 创建定时器循环
 * @param freq 最小分辨率（毫秒）
 * @param slot 时间轮槽位数量
 * @return ktimer_loop_t实例
 */
extern ktimer_loop_t* ktimer_loop_create(time_t freq, int slot);

/*
 * 销毁定时器循环
 * @return ktimer_loop_t实例
 */
extern void ktimer_loop_destroy(ktimer_loop_t* ktimer_loop);

/*
 * 检查定时器超时，如果超时调用定时器回调
 * @return ktimer_loop_t实例
 * @return 定时器超时的数量
 */
extern int ktimer_loop_run_once(ktimer_loop_t* ktimer_loop);

/*
 * 循环检查定时器超时，调用ktimer_loop_exit()退出
 * @return ktimer_loop_t实例
 */
extern void ktimer_loop_run(ktimer_loop_t* ktimer_loop);

/*
 * 退出ktimer_loop_run()
 * @return ktimer_loop_t实例
 */
extern void ktimer_loop_exit(ktimer_loop_t* ktimer_loop);

/*
 * 创建一个定时器
 * @param ktimer_loop_t实例
 * @return kktimer_t实例
 */
extern kktimer_t* ktimer_create(ktimer_loop_t* ktimer_loop);

/*
 * 停止并销毁定时器
 * @param ktimer_loop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int ktimer_stop(kktimer_t* timer);

/*
 * 取得ktimer_loop_t实例
 * @param timer kktimer_t实例
 * @return ktimer_loop_t实例
 */
extern ktimer_loop_t* ktimer_get_loop(kktimer_t* timer);

/*
 * 启动一个无限次数的定时器
 * @param timer kktimer_t实例
 * @param cb 超时回调函数
 * @param data 回调函数参数
 * @param ms 定时器超时间隔
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int ktimer_start(kktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms);

/*
 * 启动一个只超时一次的定时器，超时后将自动销毁
 * @param timer kktimer_t实例
 * @param cb 超时回调函数
 * @param data 回调函数参数
 * @param ms 定时器超时间隔
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int ktimer_start_once(kktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms);

/*
 * 启动一个有限次数的定时器，达到times次数后将自动销毁
 * @param timer kktimer_t实例
 * @param cb 超时回调函数
 * @param data 回调函数参数
 * @param ms 定时器超时间隔
 * @param times 次数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int ktimer_start_times(kktimer_t* timer, ktimer_cb_t cb, void* data, time_t ms, int times);

/*
 * 检查定时器是否在回调函数返回即将被销毁
 * @param timer kktimer_t实例
 * @retval 0 不是
 * @retval 非零 是
 */
extern int ktimer_check_dead(kktimer_t* timer);

#endif /* TIMER_API_H */
