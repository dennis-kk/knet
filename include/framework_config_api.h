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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FRAMEWORK_CONFIG_API_H
#define FRAMEWORK_CONFIG_API_H

/**
 * @defgroup framework_config 框架配置
 * 框架配置
 *
 * <pre>
 * 1. 提供kframework_t内建立的监听器、连接器配置接口
 * 2. 设置工作线程的数量，工作线程将实际负载管道
 * 
 * 监听器 - 启动前建立:
 *     kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
 *     knet_framework_acceptor_config_set_local_address(ac, "127.0.0.1", 80);
 *     knet_framework_acceptor_config_set_client_cb(ac, cb);
 *     ......
 *     knet_framework_start(...);
 *     ......
 *
 * 连接器 - 启动前建立:
 *     kframework_connector_config_t* cc = knet_framework_config_new_connector(c);
 *     knet_framework_connector_config_set_remote_address(cc, "127.0.0.1", 80);
 *     knet_framework_connector_config_set_cb(cc, cb);
 *     ......
 *     knet_framework_start(...);
 *     ......
 *
 * 监听器 - 启动后建立:
 *     kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
 *     knet_framework_acceptor_config_set_local_address(ac, "127.0.0.1", 80);
 *     knet_framework_acceptor_config_set_client_cb(ac, cb);
 *     knet_framework_acceptor_start(..., ac);
 *
 * 连接器 - 启动后建立:
 *     kframework_connector_config_t* cc = knet_framework_config_new_connector(c);
 *     knet_framework_connector_config_set_remote_address(cc, "127.0.0.1", 80);
 *     knet_framework_connector_config_set_cb(cc, cb);
 *     knet_framework_connector_start(..., cc);
 *
 * </pre>
 * @{
 */

/**
 * 从文件加载配置
 * @param c kframework_config_t实例
 * @param path 文件路径
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_framework_config_load_config_file(kframework_config_t* c, const char* path);

/**
 * 从配置字符串加载配置
 * @param c kframework_config_t实例
 * @param config_string 配置字符串
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_framework_config_load_config_string(kframework_config_t* c, const char* config_string);

/**
 * 设置工作线程数量，默认为单线程
 * @param c kframework_config_t实例
 * @param worker_thread_count 工作线程数量
 */
extern void knet_framework_config_set_worker_thread_count(
    kframework_config_t* c, int worker_thread_count);

/**
 * 设置工作线程内定时器分辨率
 * @param c kframework_config_t实例
 * @param freq 工作线程内定时器分辨率（毫秒）
 */
extern void knet_framework_config_set_worker_timer_freq(
    kframework_config_t* c, time_t freq);

/**
 * 设置工作线程内定时器槽位数量
 * @param c kframework_config_t实例
 * @param slot 定时器槽位数量
 */
extern void knet_framework_config_set_worker_timer_slot(
    kframework_config_t* c, int slot);

/**
 * 建立监听器配置
 * @param c kframework_config_t实例
 * @return kframework_acceptor_config_t实例
 */
extern kframework_acceptor_config_t* knet_framework_config_new_acceptor(
    kframework_config_t* c);

/**
 * 建立连接器配置
 * @param c kframework_config_t实例
 * @return kframework_connector_config_t实例
 */
extern kframework_connector_config_t* knet_framework_config_new_connector(
    kframework_config_t* c);

/**
 * 设置监听器本地地址
 * @param c kframework_acceptor_config_t实例
 * @param ip IP
 * @param port 地址
 */
extern void knet_framework_acceptor_config_set_local_address(
    kframework_acceptor_config_t* c, const char* ip, int port);

/**
 * 设置监听器等待队列长度
 * @param c kframework_acceptor_config_t实例
 * @param backlog 等待队列长度
 */
extern void knet_framework_acceptor_config_set_backlog(
    kframework_acceptor_config_t* c, int backlog);

/**
 * 设置客户端心跳超时
 * @param c kframework_acceptor_config_t实例
 * @param timeout 客户端心跳超时(秒)
 */
extern void knet_framework_acceptor_config_set_client_heartbeat_timeout(
    kframework_acceptor_config_t* c, int timeout);

/**
 * 设置客户端回调函数
 * @param c kframework_acceptor_config_t实例
 * @param cb 客户端回调函数
 */
extern void knet_framework_acceptor_config_set_client_cb(
    kframework_acceptor_config_t* c, knet_channel_ref_cb_t cb);

/**
 * 设置客户端发送链表最大长度
 * @param c kframework_acceptor_config_t实例
 * @param max_send_list_count 发送链表最大长度(字节)
 */
extern void knet_framework_acceptor_config_set_client_max_send_list_count(
    kframework_acceptor_config_t* c, int max_send_list_count);

/**
 * 设置客户端接收缓冲区最大长度
 * @param c kframework_acceptor_config_t实例
 * @param max_recv_buffer_length 接收缓冲区最大长度(字节)
 */
extern void knet_framework_acceptor_config_set_client_max_recv_buffer_length(
    kframework_acceptor_config_t* c, int max_recv_buffer_length);

/**
 * 设置连接器需要连接的地址
 * @param c kframework_connector_config_t实例
 * @param ip IP
 * @param port 端口
 */
extern void knet_framework_connector_config_set_remote_address(
    kframework_connector_config_t* c, const char* ip, int port);

/**
 * 设置连接器心跳超时
 * @param c kframework_connector_config_t实例
 * @param timeout 客户端心跳超时(秒)
 */
extern void knet_framework_connector_config_set_heartbeat_timeout(
    kframework_connector_config_t* c, int timeout);

/**
 * 设置连接器连接超时
 * @param c kframework_connector_config_t实例
 * @param timeout 连接超时(秒)
 */
extern void knet_framework_connector_config_set_connect_timeout(
    kframework_connector_config_t* c, int timeout);

/**
 * 设置连接器自动重连
 * @param c kframework_connector_config_t实例
 * @param auto_reconnect 自动重连标志
 */
extern void knet_framework_connector_config_set_auto_reconnect(
    kframework_connector_config_t* c, int auto_reconnect);

/**
 * 设置连接器回调函数
 * @param c kframework_connector_config_t实例
 * @param cb 连接器回调函数
 */
extern void knet_framework_connector_config_set_cb(
    kframework_connector_config_t* c, knet_channel_ref_cb_t cb);

/**
 * 设置连接器发送链表最大长度
 * @param c kframework_connector_config_t实例
 * @param max_send_list_count 发送链表最大长度(字节)
 */
extern void knet_framework_connector_config_set_client_max_send_list_count(
    kframework_connector_config_t* c, int max_send_list_count);

/**
 * 设置连接器接收缓冲区最大长度
 * @param c kframework_connector_config_t实例
 * @param max_recv_buffer_length 接收缓冲区最大长度(字节)
 */
extern void knet_framework_connector_config_set_client_max_recv_buffer_length(
    kframework_connector_config_t* c, int max_recv_buffer_length);

/** @} */

#endif /* FRAMEWORK_CONFIG_API_H */
