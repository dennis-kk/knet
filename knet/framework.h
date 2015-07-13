#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "framework_api.h"

/**
 * 建立框架配置器
 * @return framework_config_t实例
 */
framework_config_t* framework_config_create();

/**
 * 销毁框架配置器
 * @param c framework_config_t实例
 */
void framework_config_destroy(framework_config_t* c);

/**
 * 获取IP
 * @param c framework_config_t实例
 * @return IP
 */
const char* framework_config_get_ip(framework_config_t* c);

/**
 * 获取监听端口
 * @param c framework_config_t实例
 * @return 监听端口
 */
int framework_config_get_port(framework_config_t* c);

/**
 * 获取backlog
 * @param c framework_config_t实例
 * @return backlog
 */
int framework_config_get_backlog(framework_config_t* c);

/**
 * 获取工作线程数量
 * @param c framework_config_t实例
 * @return 工作线程数量
 */
int framework_config_get_worker_thread_count(framework_config_t* c);

/**
 * 获取发送链表最大元素数
 * @param c framework_config_t实例
 * @return 发送链表最大元素数
 */
int framework_config_get_max_send_list(framework_config_t* c);

/**
 * 获取接收缓冲区最大长度
 * @param c framework_config_t实例
 * @return 接收缓冲区最大长度
 */
int framework_config_get_max_recv_buffer(framework_config_t* c);

/**
 * 事件channel_cb_event_accept处理函数
 * @param channel 监听管道，channel_ref_t实例
 * @param e 管道事件
 */
void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e);

/**
 * 获取负载均衡器
 * @param f framework_t实例
 * @return loop_balancer_t实例
 */
loop_balancer_t* framework_get_balancer(framework_t* f);

/**
 * 获取管道回调
 * @param f framework_t实例
 * @return 管道回调函数指针
 */
channel_ref_cb_t framework_get_cb(framework_t* f);

#endif /* FRAMEWORK_H */
