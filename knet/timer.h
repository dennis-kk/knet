#ifndef TIMER_H
#define TIMER_H

#include "config.h"
#include "timer_api.h"

/*
 * 销毁定时器
 * @param timer ktimer_t实例
 */
void ktimer_destroy(ktimer_t* timer);

/*
 * 检查定时器是否超时
 * @param timer ktimer_t实例
 * @param ms 当前时间戳
 * @retval 0 没有超时
 * @retval 非零 超时
 */
int ktimer_check_timeout(ktimer_t* timer, time_t ms);

/*
 * 设置定时器所在的当前链表
 * @param timer ktimer_t实例
 * @param list 链表
 */
void ktimer_set_current_list(ktimer_t* timer, dlist_t* list);

/*
 * 取得当前所属链表
 * @param timer ktimer_t实例
 * @return dlist_t实例
 */
dlist_t* ktimer_get_current_list(ktimer_t* timer);

/*
 * 设置定时器所在的当前链表的链表节点
 * @param timer ktimer_t实例
 * @param node 链表节点
 */
void ktimer_set_current_list_node(ktimer_t* timer, dlist_node_t* node);

/*
 * 取得当前链表节点
 * @param timer ktimer_t实例
 * @return dlist_node_t实例
 */
dlist_node_t* ktimer_get_current_list_node(ktimer_t* timer);

/*
 * 取得滴答间隔
 * @param ktimer_loop ktimer_loop_t实例
 * @return 滴答间隔
 */
time_t ktimer_loop_get_tick_intval(ktimer_loop_t* ktimer_loop);

#endif /* TIMER_H */
