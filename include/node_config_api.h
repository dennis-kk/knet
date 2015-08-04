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

#ifndef NODE_CONFIG_API_H
#define NODE_CONFIG_API_H

#include "config.h"

/**
 * 设置节点身份
 * @param c knode_config_t实例
 * @param type 节点类型
 * @param id 节点ID
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_identity(knode_config_t* c, uint32_t type, uint32_t id);

/**
 * 设置节点关注的节点类型
 *
 * <pre>
 * 节点类型为uint32_t类型的有效非零无符号整数值，可变参数的最后一个参数必须为0
 * </pre>
 * @param c knode_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_concern_type(knode_config_t* c, ...);

/**
 * 设置节点工作线程数量
 * @param c knode_config_t实例
 * @param worker_count 工作线程数量，默认为一个工作线程
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern void knet_node_config_set_worker_thread_count(knode_config_t* c, int worker_count);

/**
 * 设置节点本地监听地址，其他节点可以通过这个地址主动连接本节点
 * @param c knode_config_t实例
 * @param ip IP
 * @param port 端口
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_address(knode_config_t* c, const char* ip, int port);

/**
 * 设置根节点的监听地址，如果本地监听地址与根节点监听地址相同则本节点为根节点
 *
 * <pre>
 * 也可以通过调用knet_node_config_set_root将本节点设置为根节点，同一个集群内可以有多个根节点，但每个
 * 根节点都认为自己是唯一的根节点
 * </pre>
 * @param c knode_config_t实例
 * @param ip IP
 * @param port 端口
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_root_address(knode_config_t* c, const char* ip, int port);

/**
 * 设置节点为根节点
 * @param c knode_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_root(knode_config_t* c);

/**
 * 设置本节点是否为根节点
 * @param c knode_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_check_root(knode_config_t* c);

/**
 * 设置节点事件处理函数
 * @param c knode_config_t实例
 * @param cb 事件处理函数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_node_cb(knode_config_t* c, knet_node_cb_t cb);

/**
 * 设置节点监控地址，所有连接到这个监听地址的连接都会收到一条文本汇报信息并立即断开连接
 * @param c knode_config_t实例
 * @param ip IP
 * @param port 端口
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_monitor_address(knode_config_t* c, const char* ip, int port);

/**
 * 设置节点管理监听地址，连接到本监听地址的连接将保持长连接
 * @param c knode_config_t实例
 * @param ip IP
 * @param port 端口
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_manage_address(knode_config_t* c, const char* ip, int port);

/**
 * 设置节点管理命令处理函数
 * @param c knode_config_t实例
 * @param cb 命令处理函数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_manage_cb(knode_config_t* c, knet_node_manage_cb_t cb);

/**
 * 设置节点监控回调
 * @param c knode_config_t实例
 * @param cb 节点监控回调函数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_monitor_cb(knode_config_t* c, knet_node_monitor_cb_t cb);

/**
 * 开启/关闭节点退出自动保存IP黑名单
 * @param c knode_config_t实例
 * @param on 零表示关闭，非零表示开启
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_black_ip_filter_auto_save_at_exit(knode_config_t* c, int on);

/**
 * 设置IP黑名单文件路径
 * @param c knode_config_t实例
 * @param path 黑名单文件路径
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_black_ip_filter_file_path(knode_config_t* c, const char* path);

/**
 * 开启/关闭节点退出自动保存IP白名单
 * @param c knode_config_t实例
 * @param on 零表示关闭，非零表示开启
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_white_ip_filter_auto_save_at_exit(knode_config_t* c, int on);

/**
 * 设置IP白名单文件路径
 * @param c knode_config_t实例
 * @param path 白名单文件路径
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_config_set_white_ip_filter_file_path(knode_config_t* c, const char* path);

/**
 * 设置节点管道接收缓冲区最大长度
 * @param c knode_config_t实例
 * @param max_recv_buffer_length 管道接收缓冲区最大长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern void knet_node_config_set_node_channel_max_recv_buffer_length(knode_config_t* c, int max_recv_buffer_length);

/**
 * 设置节点管道发送缓冲区链最大长度
 * @param c knode_config_t实例
 * @param max_send_list_count 发送缓冲区链最大长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern void knet_node_config_set_node_channel_max_send_list_count(knode_config_t* c, int max_send_list_count);

/**
 * 设置节点管道空闲超时（秒）
 * @param c knode_config_t实例
 * @param timeout 管道空闲超时（秒）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern void knet_node_config_set_node_channel_idle_timeout(knode_config_t* c, int timeout);

/**
 * 取得框架配置
 * @param c knode_config_t实例
 * @return kframework_config_t实例
 */
extern kframework_config_t* knet_node_config_get_framework_config(knode_config_t* c);

/**
 * 设置管理命令结果的缓冲区最大长度
 * @param c knode_config_t实例
 * @param max_output_buffer_length 缓冲区最大长度
 */
extern void knet_node_config_set_manage_max_output_buffer_length(knode_config_t* c, int max_output_buffer_length);

#endif /* NODE_CONFIG_API_H */
