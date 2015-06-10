#ifndef TIMER_H
#define TIMER_H

#include "config.h"
#include "timer_api.h"

/*
 * WARNING 未经测试，需要等测试完成
 */

/*
 * 添加定时器到循环
 * @param timer_loop timer_loop_t实例
 * @param timer timer_t实例
 * @param ms 定时器超时间隔
 */
void timer_loop_add_timer(timer_loop_t* timer_loop, timer_t* timer, time_t ms);

/*
 * 销毁定时器
 * @param timer timer_t实例
 */
void timer_destroy(timer_t* timer);

/*
 * 检查定时器是否超时
 * @param timer timer_t实例
 * @param ms 当前时间戳
 * @retval 0 没有超时
 * @retval 非零 超时
 */
int timer_check_timeout(timer_t* timer, time_t ms);

/*
 * 设置定时器所在的当前链表
 * @param timer timer_t实例
 * @param list 链表
 */
void timer_set_current_list(timer_t* timer, dlist_t* list);

dlist_t* timer_get_current_list(timer_t* timer);
dlist_node_t* timer_get_current_list_node(timer_t* timer);

/*
 * 设置定时器所在的当前链表的链表节点
 * @param timer timer_t实例
 * @param node 链表节点
 */
void timer_set_current_list_node(timer_t* timer, dlist_node_t* node);

#endif /* TIMER_H */
