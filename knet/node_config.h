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

#ifndef NODE_CONFIG_H
#define NODE_CONFIG_H

#include "config.h"
#include "node_config_api.h"

/**
 * 建立节点配置
 * @param node knode_t实例
 * @return knode_config_t实例
 */
knode_config_t* knet_node_config_create(knode_t* node);

/**
 * 销毁节点配置
 * @param c knode_config_t实例
 */
void knet_node_config_destroy(knode_config_t* c);

/**
 * 取得本节点类型
 * @param c knode_config_t实例
 * @return 节点类型
 */
uint32_t knet_node_config_get_type(knode_config_t* c);

/**
 * 取得本节点类型
 * @param c knode_config_t实例
 * @return 节点类型
 */
uint32_t knet_node_config_get_id(knode_config_t* c);

/**
 * 取得节点工作线程数量
 * @param c knode_config_t实例
 * @return 工作线程数量
 */
int knet_node_config_get_worker_thread_count(knode_config_t* c);

/**
 * 取得节点监听IP
 * @param c knode_config_t实例
 * @return 监听IP
 */
const char* knet_node_config_get_ip(knode_config_t* c);

/**
 * 取得节点监听端口
 * @param c knode_config_t实例
 * @return 监听端口
 */
int knet_node_config_get_port(knode_config_t* c);

/**
 * 取得根节点监听IP
 * @param c knode_config_t实例
 * @return 根节点监听IP
 */
const char* knet_node_config_get_root_ip(knode_config_t* c);

/**
 * 取得根节点监听端口
 * @param c knode_config_t实例
 * @return 根节点监听端口
 */
int knet_node_config_get_root_port(knode_config_t* c);

/**
 * 取得节点事件处理函数
 * @param c knode_config_t实例
 * @return 节点事件处理函数
 */
knet_node_cb_t knet_node_config_get_node_cb(knode_config_t* c);

/**
 * 取得节点监控IP
 * @param c knode_config_t实例
 * @return 节点监控IP
 */
const char* knet_node_config_get_monitor_ip(knode_config_t* c);

/**
 * 取得节点监控端口
 * @param c knode_config_t实例
 * @return 节点监控端口
 */
int knet_node_config_get_monitor_port(knode_config_t* c);

/**
 * 取得节点管理IP
 * @param c knode_config_t实例
 * @return 节点管理IP
 */
const char* knet_node_config_get_manage_ip(knode_config_t* c);

/**
 * 取得节点管理端口
 * @param c knode_config_t实例
 * @return 节点管理端口
 */
int knet_node_config_get_manage_port(knode_config_t* c);

/**
 * 取得节点管理命令处理函数指针
 * @param c knode_config_t实例
 * @return 管理命令处理函数指针
 */
knet_node_manage_cb_t knet_node_config_get_manage_cb(knode_config_t* c);

/**
 * 取得节点监控处理函数指针
 * @param c knode_config_t实例
 * @return 监控处理函数指针
 */
knet_node_monitor_cb_t knet_node_config_get_monitor_cb(knode_config_t* c);

/**
 * 取得节点IP黑名单退出自动保存开关
 * @param c knode_config_t实例
 * @return IP黑名单退出自动保存开关
 */
int knet_node_config_get_black_ip_filter_auto_save_at_exit(knode_config_t* c);

/**
 * 取得节点IP黑名单文件路径
 * @param c knode_config_t实例
 * @return IP黑名单文件路径
 */
const char* knet_node_config_get_black_ip_filter_file_path(knode_config_t* c);

/**
 * 取得节点IP白名单退出自动保存开关
 * @param c knode_config_t实例
 * @return IP白名单退出自动保存开关
 */
int knet_node_config_get_white_ip_filter_auto_save_at_exit(knode_config_t* c);

/**
 * 取得节点IP白名单文件路径
 * @param c knode_config_t实例
 * @return IP白名单文件路径
 */
const char* knet_node_config_get_white_ip_filter_file_path(knode_config_t* c);

/**
 * 检查节点类型是否为关注类型
 * @param c knode_config_t实例
 * @param type 节点类型
 * @retval 零 不是
 * @retval 非零 是
 */
int knet_node_config_concern(knode_config_t* c, int type);

/**
 * 取得节点管道接收缓冲区最大长度（字节）
 * @param c knode_config_t实例
 * @return 节点管道接收缓冲区最大长度
 */
int knet_node_config_get_node_channel_max_recv_buffer_length(knode_config_t* c);

/**
 * 取得节点管道发送缓冲区链最大长度（字节）
 * @param c knode_config_t实例
 * @return 节点管道发送缓冲区链最大长度
 */
int knet_node_config_get_node_channel_max_send_list_count(knode_config_t* c);

/**
 * 取得节点管道空闲超时（秒）
 * @param c knode_config_t实例
 * @return 节点管道空闲超时（秒）
 */
int knet_node_config_get_node_channel_idle_timeout(knode_config_t* c);

/**
 * 取得管理命令结果缓冲区最大长度（字节）
 * @param c knode_config_t实例
 * @return 管理命令结果缓冲区最大长度（字节）
 */
int knet_node_config_get_manage_max_output_buffer_length(knode_config_t* c);

/**
 * 取得用户指针
 * @param c knode_config_t实例
 * @return 用户指针
 */
void* knet_node_config_get_ptr(knode_config_t* c);

#endif /* NODE_CONFIG_H */
