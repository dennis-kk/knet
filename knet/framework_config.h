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

#ifndef FRAMEWORK_CONFIG_H
#define FRAMEWORK_CONFIG_H

#include "config.h"
#include "framework_config_api.h"

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
 * 取得工作线程数量
 * @param c framework_config_t实例
 * @return 工作线程数量
 */
int framework_config_get_worker_thread_count(framework_config_t* c);

/**
 * 取得监听器配置链表
 * @param c framework_config_t实例
 * @return 监听器配置链表
 */
dlist_t* framework_config_get_acceptor_config(framework_config_t* c);

/**
 * 取得连接器配置链表
 * @param c framework_config_t实例
 * @return 连接器配置链表
 */
dlist_t* framework_config_get_connector_config(framework_config_t* c);

/**
 * 取得监听器IP
 * @param c framework_acceptor_config_t实例
 * @return 监听器IP
 */
const char* framework_acceptor_config_get_ip(framework_acceptor_config_t* c);

/**
 * 取得监听器端口
 * @param c framework_acceptor_config_t实例
 * @return 监听器端口
 */
int framework_acceptor_config_get_port(framework_acceptor_config_t* c);

/**
 * 取得监听器等待队列长度
 * @param c framework_acceptor_config_t实例
 * @return 监听器等待队列长度
 */
int framework_acceptor_config_get_backlog(framework_acceptor_config_t* c);

/**
 * 取得监听器客户端心跳(秒)
 * @param c framework_acceptor_config_t实例
 * @return 客户端心跳(秒)
 */
int framework_acceptor_config_get_client_heartbeat_timeout(framework_acceptor_config_t* c);

/**
 * 取得监听器客户端回调函数
 * @param c framework_acceptor_config_t实例
 * @return 客户端回调函数
 */
channel_ref_cb_t framework_acceptor_config_get_client_cb(framework_acceptor_config_t* c);

/**
 * 取得监听器客户端发送队列最大长度
 * @param c framework_acceptor_config_t实例
 * @return 客户端发送队列最大长度
 */
int framework_acceptor_config_get_client_max_send_list_count(framework_acceptor_config_t* c);

/**
 * 取得监听器客户端接收缓冲区最大长度
 * @param c framework_acceptor_config_t实例
 * @return 客户端接收缓冲区最大长度
 */
int framework_acceptor_config_get_client_max_recv_buffer_length(framework_acceptor_config_t* c);

/**
 * 取得连接器对端IP
 * @param c framework_connector_config_t实例
 * @return 连接器对端IP
 */
const char* framework_connector_config_get_remote_ip(framework_connector_config_t* c);

/**
 * 取得连接器对端端口
 * @param c framework_connector_config_t实例
 * @return 连接器对端端口
 */
int framework_connector_config_get_remote_port(framework_connector_config_t* c);

/**
 * 取得连接器心跳超时(秒)
 * @param c framework_connector_config_t实例
 * @return 连接器心跳超时(秒)
 */
int framework_connector_config_get_heartbeat_timeout(framework_connector_config_t* c);

/**
 * 取得连接器连接超时(秒)
 * @param c framework_connector_config_t实例
 * @return 连接器连接超时(秒)
 */
int framework_connector_config_get_connect_timeout(framework_connector_config_t* c);

/**
 * 取得连接器回调函数
 * @param c framework_connector_config_t实例
 * @return 连接器回调函数
 */
channel_ref_cb_t framework_connector_config_get_cb(framework_connector_config_t* c);

/**
 * 取得连接器发送链表最大长度
 * @param c framework_connector_config_t实例
 * @return 连接器发送链表最大长度
 */
int framework_connector_config_get_max_send_list_count(framework_connector_config_t* c);

/**
 * 取得连接器接收缓冲区最大长度
 * @param c framework_connector_config_t实例
 * @return 连接器接收缓冲区最大长度
 */
int framework_connector_config_get_max_recv_buffer_length(framework_connector_config_t* c);

#endif /* FRAMEWORK_CONFIG_H */
