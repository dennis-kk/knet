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

#ifndef FRAMEWORK_CONFIG_API_H
#define FRAMEWORK_CONFIG_API_H

/**
 * @defgroup framework_config 框架配置
 * 框架配置
 *
 * <pre>
 * 1. 提供framework_t内建立的监听器、连接器配置接口
 * 2. 设置工作线程的数量，工作线程将实际负载管道
 * 
 * 监听器 - 启动前建立:
 *     framework_acceptor_config_t* ac = framework_config_new_acceptor(c);
 *     framework_acceptor_config_set_local_address(ac, "127.0.0.1", 80);
 *     framework_acceptor_config_set_client_cb(ac, cb);
 *     ......
 *     framework_start(...);
 *     ......
 *
 * 连接器 - 启动前建立:
 *     framework_connector_config_t* cc = framework_config_new_connector(c);
 *     framework_connector_config_set_remote_address(cc, "127.0.0.1", 80);
 *     framework_connector_config_set_cb(cc, cb);
 *     ......
 *     framework_start(...);
 *     ......
 *
 * 监听器 - 启动后建立:
 *     framework_acceptor_config_t* ac = framework_config_new_acceptor(c);
 *     framework_acceptor_config_set_local_address(ac, "127.0.0.1", 80);
 *     framework_acceptor_config_set_client_cb(ac, cb);
 *     framework_acceptor_start(..., ac);
 *
 * 连接器 - 启动后建立:
 *     framework_connector_config_t* cc = framework_config_new_connector(c);
 *     framework_connector_config_set_remote_address(cc, "127.0.0.1", 80);
 *     framework_connector_config_set_cb(cc, cb);
 *     framework_connector_start(..., cc);
 *
 * </pre>
 * @{
 */

/**
 * 设置工作线程数量，默认为单线程
 * @param c framework_config_t实例
 * @param worker_thread_count 工作线程数量
 */
extern void framework_config_set_worker_thread_count(framework_config_t* c, int worker_thread_count);

/**
 * 建立监听器配置
 * @param c framework_config_t实例
 * @return framework_acceptor_config_t实例
 */
extern framework_acceptor_config_t* framework_config_new_acceptor(framework_config_t* c);

/**
 * 建立连接器配置
 * @param c framework_config_t实例
 * @return framework_connector_config_t实例
 */
extern framework_connector_config_t* framework_config_new_connector(framework_config_t* c);

/**
 * 设置监听器本地地址
 * @param c framework_acceptor_config_t实例
 * @param ip IP
 * @param port 地址
 */
extern void framework_acceptor_config_set_local_address(framework_acceptor_config_t* c, const char* ip, int port);

/**
 * 设置监听器等待队列长度
 * @param c framework_acceptor_config_t实例
 * @param backlog 等待队列长度
 */
extern void framework_acceptor_config_set_backlog(framework_acceptor_config_t* c, int backlog);

/**
 * 设置客户端心跳超时
 * @param c framework_acceptor_config_t实例
 * @param timeout 客户端心跳超时(秒)
 */
extern void framework_acceptor_config_set_client_heartbeat_timeout(framework_acceptor_config_t* c, int timeout);

/**
 * 设置客户端回调函数
 * @param c framework_acceptor_config_t实例
 * @param cb 客户端回调函数
 */
extern void framework_acceptor_config_set_client_cb(framework_acceptor_config_t* c, channel_ref_cb_t cb);

/**
 * 设置客户端发送链表最大长度
 * @param c framework_acceptor_config_t实例
 * @param max_send_list_count 发送链表最大长度(字节)
 */
extern void framework_acceptor_config_set_client_max_send_list_count(framework_acceptor_config_t* c, int max_send_list_count);

/**
 * 设置客户端接收缓冲区最大长度
 * @param c framework_acceptor_config_t实例
 * @param max_recv_buffer_length 接收缓冲区最大长度(字节)
 */
extern void framework_acceptor_config_set_client_max_recv_buffer_length(framework_acceptor_config_t* c, int max_recv_buffer_length);

/**
 * 设置连接器需要连接的地址
 * @param c framework_connector_config_t实例
 * @param ip IP
 * @param port 端口
 */
extern void framework_connector_config_set_remote_address(framework_connector_config_t* c, const char* ip, int port);

/**
 * 设置连接器心跳超时
 * @param c framework_connector_config_t实例
 * @param timeout 客户端心跳超时(秒)
 */
extern void framework_connector_config_set_heartbeat_timeout(framework_connector_config_t* c, int timeout);

/**
 * 设置连接器连接超时
 * @param c framework_connector_config_t实例
 * @param timeout 连接超时(秒)
 */
extern void framework_connector_config_set_connect_timeout(framework_connector_config_t* c, int timeout);

/**
 * 设置连接器回调函数
 * @param c framework_connector_config_t实例
 * @param cb 连接器回调函数
 */
extern void framework_connector_config_set_cb(framework_connector_config_t* c, channel_ref_cb_t cb);

/**
 * 设置连接器发送链表最大长度
 * @param c framework_connector_config_t实例
 * @param max_send_list_count 发送链表最大长度(字节)
 */
extern void framework_connector_config_set_client_max_send_list_count(framework_connector_config_t* c, int max_send_list_count);

/**
 * 设置连接器接收缓冲区最大长度
 * @param c framework_connector_config_t实例
 * @param max_recv_buffer_length 接收缓冲区最大长度(字节)
 */
extern void framework_connector_config_set_client_max_recv_buffer_length(framework_connector_config_t* c, int max_recv_buffer_length);

/** @} */

#endif /* FRAMEWORK_CONFIG_API_H */
