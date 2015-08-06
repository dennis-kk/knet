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
 * @defgroup node 节点
 * 节点
 *
 * <pre>
 * 节点提供了如下几方面的功能：
 * 1. 组网
 * 2. 监控，管理
 * 3. IP黑名单/白名单
 * 
 * <b>组网</b>
 * 节点分为根节点和普通节点两种类型，根节点作为组网的通知者存在，组网过程如下：
 * 1. 普通节点可以选择连接根节点（通过配置），普通节点可以设置自己所关注的其他节点的类型(node-type)
 * 2. 连接到根节点后汇报自己的身份{IP, port, node-type, node-id}
 * 3. 根节点推送新加入的节点的身份信息给所有已经在节点集群内的其他节点
 * 4. 接收到根节点推送过来的身份信息后，如果是自己关注的节点类型，那么转到步骤5，否则不做处理
 * 5. 主动连接新加入的节点并汇报自己的身份，新加入的节点同时也推送自己的身份给连接自己的节点
 *
 * <b>监控</b>
 * 每个节点提供了一个监控端口和一个管理端口.
 * 1. 监控端口
 *    监控端口可以被任何监控客户端连接，如果有连接建立，节点会回调用户设置的监控回调函数，回调函数
 *    退出后连接将被断开.
 * 2. 管理端口
 *    管理端口通常应开启在对内网的网卡上（或者连接受保护的地址上），不应该被外网访问到. 如果有监控
 *    客户端连接，那么监控客户端可以发送命令给用户设置的回调函数处理，命令为一个字符串以并且以回车换行结尾,
 *    可以同时发送多条命令. 回调函数可以通过返回值来告诉节点后续的处理方式: 保持连接或命令处理完成后断开
 *    连接.
 *
 * <b>IP黑名单/白名单</b>
 * 如果用户提供两个IP名单文件（通过设置），那么就自动开启了IP名单过滤，过滤规则为：
 * 1. 白名单优先
 * 2. 不在白名单的IP可以被黑名单过滤
 *
 * 具体设置可参见节点配置.
 * </pre>
 * @{
 */

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
 * 启动节点，使用命令行传递的参数
 *
 * <pre>
 * 命令行参数为
 * -root ip:port
 * -self ip:port:node-type:node-id
 * </pre>
 * @param node knode_t实例
 * @param argc 命令行参数个数
 * @param argv 命令行参数数组
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_start_argv(knode_t* node, int argc, char** argv);

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

/**
 * 取得本机节点
 * @param proxy knode_proxy_t实例
 * @return 本机节点
 */
extern knode_t* knet_node_proxy_get_self(knode_proxy_t* proxy);

/**
 * 发送到节点
 * @param proxy knode_proxy_t实例
 * @param buffer 数据
 * @param size 长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_proxy_write(knode_proxy_t* proxy, void* buffer, int size);

/**
 * 读取节点数据
 * @param proxy knode_proxy_t实例
 * @param buffer 数据
 * @param size 长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_proxy_read(knode_proxy_t* proxy, void* buffer, int size);

/**
 * 读取节点数据，但不清除
 * @param proxy knode_proxy_t实例
 * @param buffer 数据
 * @param size 长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_proxy_copy(knode_proxy_t* proxy, void* buffer, int size);

/**
 * 可读取数据的长度（字节）
 * @param proxy knode_proxy_t实例
 * @return 可读取数据的长度（字节）
 */
extern int knet_node_proxy_available(knode_proxy_t* proxy);

/**
 * 断开节点连接
 * @param proxy knode_proxy_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_node_proxy_close(knode_proxy_t* proxy);

/** @} */

#endif /* NODE_API_H */
