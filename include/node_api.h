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

#ifndef NODE_API_H
#define NODE_API_H

#include "config.h"

/**
 * 创建一个新的节点
 * @return knode_t实例
 */
extern knode_t* knet_node_create();

/**
 * 销毁节点
 * @param node knode_t实例
 */
extern void knet_node_destroy(knode_t* node);

/**
 * 销毁节点
 * @param node knode_t实例
 */
extern knode_config_t* knet_node_get_config(knode_t* node);

/**
 * 启动节点
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_start(knode_t* node);

/**
 * 关闭节点
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_stop(knode_t* node);

/**
 * 等待节点关闭
 * @param node knode_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern void knet_node_wait_for_stop(knode_t* node);

/**
 * 广播数据到所有与本节点有连接的节点
 * @param node knode_t实例
 * @param msg 数据包
 * @param size 数据长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_broadcast(knode_t* node, const void* msg, int size);

/**
 * 广播数据到所有与本节点有连接的节点的指定类型节点
 * @param node knode_t实例
 * @param type 节点类型
 * @param msg 数据包
 * @param size 数据长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_broadcast_by_type(knode_t* node, uint32_t type, const void* msg, int size);

/**
 * 发送数据到指定ID的节点
 * @param node knode_t实例
 * @param id 节点ID
 * @param msg 数据包
 * @param size 数据长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_write(knode_t* node, uint32_t id, const void* msg, int size);

/**
 * 取得框架
 * @param node knode_t实例
 * @return kframework_t实例
 */
extern kframework_t* knet_node_get_framework(knode_t* node);

/**
 * 取得IP黑名单
 * @param node knode_t实例
 * @return kip_filter_t实例
 */
extern kip_filter_t* knet_node_get_black_ip_filter(knode_t* node);

/**
 * 取得IP白名单
 * @param node knode_t实例
 * @return kip_filter_t实例
 */
extern kip_filter_t* knet_node_get_white_ip_filter(knode_t* node);

/**
 * 取得节点代理ID
 *
 * 节点代理ID既远程节点的ID
 * @param proxy knode_proxy_t实例
 * @return 节点代理ID
 */
extern uint32_t knet_node_proxy_get_id(knode_proxy_t* proxy);

/**
 * 取得节点代理类型
 *
 * 节点代理ID既远程节点的类型
 * @param proxy knode_proxy_t实例
 * @return 节点代理ID
 */
extern uint32_t knet_node_proxy_get_type(knode_proxy_t* proxy);

/**
 * 取得节点代理本次可以读取的数据长度，此长度代表了一次节点发送
 * @param proxy knode_proxy_t实例
 * @return 本次可以读取的数据长度
 */
extern uint32_t knet_node_proxy_get_data_length(knode_proxy_t* proxy);

#endif /* NODE_API_H */
