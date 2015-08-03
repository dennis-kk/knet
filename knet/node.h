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

#ifndef NODE_H
#define NODE_H

#include "config.h"
#include "node_api.h"

/**
 * 添加节点代理
 * @param node knode_t实例
 * @param type 节点类型
 * @param id 节点ID
 * @param channel 节点代理管道引用
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_node_add_node(knode_t* node, uint32_t type, uint32_t id, kchannel_ref_t* channel);

/**
 * 删除节点代理
 * @param node knode_t实例
 * @param channel 节点代理管道引用
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_node_remove_node_by_channel_ref(knode_t* node, kchannel_ref_t* channel);

/**
 * 删除节点代理
 * @param node knode_t实例
 * @param id 节点代理ID
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_node_remove_node_by_id(knode_t* node, uint32_t id);

/**
 * 建立节点代理
 * @return knode_proxy_t实例
 */
knode_proxy_t* node_proxy_create();

/**
 * 销毁节点代理
 * @param proxy knode_proxy_t实例
 */
void node_proxy_destroy(knode_proxy_t* proxy);

/**
 * 哈希表值销毁函数
 * @param param 节点代理指针
 */
void hash_node_channel_id_dtor(void* param);

/**
 * 启动根节点监听器
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int node_root_start(knode_t* node);

/**
 * 连接根节点
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int node_connect_root(knode_t* node);

/**
 * 启动节点监听器
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int node_local_start(knode_t* node);

/**
 * 读取IP黑/白名单文件
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int node_open_filter_files(knode_t* node);

/**
 * 保存IP黑/白名单文件
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
void node_save_filter_files(knode_t* node);

/**
 * 节点管道回调
 * @param channel kchannel_ref_t实例
 * @param e 管道事件ID
 */
void node_channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e);

/**
 * 发送节点身份验证请求
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _node_login_req(kchannel_ref_t* channel);

/**
 * 发送节点身份验证应答
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _node_login_ack(kchannel_ref_t* channel);

/**
 * 发送数据到其他节点
 * @param channel kchannel_ref_t实例
 * @param data 数据块指针
 * @param size 数据长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _node_send(kchannel_ref_t* channel, const void* data, uint32_t size);

/**
 * 根节点通知其他节点有新节点加入集群
 * @param channel kchannel_ref_t实例
 * @param ip 新加入节点IP
 * @param port 新加入节点监听端口
 * @param type 新加入节点类型
 * @param id 新加入节点ID
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _node_broadcast_join(knode_t* node, const char* ip, int port, uint32_t type, uint32_t id);

/**
 * 获取消息ID
 * @param channel kchannel_ref_t实例
 * @return 消息ID
 */
uint32_t _proc_msg_id(kchannel_ref_t* channel);

/**
 * 发送心跳请求
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _node_send_heartbeat_req(kchannel_ref_t* channel);

/**
 * 发送心跳应答
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _node_send_heartbeat_ack(kchannel_ref_t* channel);

/**
 * 身份验证请求处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_login_req(kchannel_ref_t* channel);

/**
 * 身份验证应答处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_login_ack(kchannel_ref_t* channel);

/**
 * 新节点加入通知处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_join(kchannel_ref_t* channel);

/**
 * 节点数据处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_data(kchannel_ref_t* channel);

/**
 * 节点管道成功处理函数 - 主动连接
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_connect(kchannel_ref_t* channel);

/**
 * 节点管道成功处理函数 - 监听
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_accept(kchannel_ref_t* channel);

/**
 * 节点管道数据处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_recv(kchannel_ref_t* channel);

/**
 * 节点内部协议处理入口函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_disjoin(kchannel_ref_t* channel);

/**
 * 节点管道超时处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_timeout(kchannel_ref_t* channel);

/**
 * 节点心跳请求处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_heartbeat_req(kchannel_ref_t* channel);

/**
 * 节点心跳应答处理函数
 * @param channel kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int _on_node_heartbeat_ack(kchannel_ref_t* channel);

#endif /* NODE_H */
