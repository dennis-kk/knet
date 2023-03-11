﻿/*
 * Copyright (c) 2014-2016, dennis wang
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

#ifndef CHANNEL_REF_API_H
#define CHANNEL_REF_API_H

#include "config.h"

/**
 * @defgroup 管道引用 管道引用
 * 管道引用
 *
 * <pre>
 * kchannel_ref_t作为kchannel_t的包装器，对于用户透明化了管道的内部实现，同时提供了引用计数用于
 * 管道的生命周期管理.
 *
 * 管道有3种类型：
 * 
 * 1. 连接器
 * 2. 监听器
 * 3. 由监听器接受的新管道
 *
 * 管道有3种状态:
 * 
 * 1. 新建立 刚建立但不确定是作为连接器或者监听器存在
 * 2. 活跃   已经确定了自己的角色
 * 3. 关闭   已经关闭，但还未销毁，引用计数不为零
 *
 * 在没有负载均衡器存在的情况下(kloop_t没有通过knet_loop_balancer_attach关联到kloop_balancer_t),
 * 所有连接器管道都会在当前kloop_t内运行，所有由监听器接受的管道也会在kloop_t内运行.
 * 如果kloop_t已经关联到负载均衡器，连接器/监听器接受的管道可能不在当前kloop_t内
 * 运行，负载均衡器会根据活跃管道的数量将这个管道分配到其他kloop_t运行，或者仍然在当前kloop_t内运行，
 * 结果取决于当前所有kloop_t负载的情况（活跃管道的数量）.
 *
 * 可以调用函数knet_channel_ref_check_balance确定管道是否被负载均衡调配，调用knet_channel_ref_check_state
 * 检查管道当前所处的状态，knet_channel_ref_close关闭管道，无论此时管道的引用计数是否为零，管道的套接字都会
 * 被关闭，当管道引用计数为零时，kloop_t才会真正销毁它.调用knet_channel_ref_equal可以判断两个管道引用是否
 * 指向同一个管道.
 * 
 * 可以通过调用knet_channel_ref_set_timeout设置管道的读空闲超时（秒），这可以用做心跳包的处理，调用
 * knet_channel_ref_connect时最后一个参数传递一个非零值可以设置连接器的连接超时（秒），这可以用于重连.
 * 调用knet_channel_ref_get_socket_fd得到管道套接字，调用knet_channel_ref_get_uuid的到管道UUID.
 * </pre>
 * @{
 */

/**
 * 增加管道引用计数，并创建与管道关联的新的kchannel_ref_t实例
 *
 * knet_channel_ref_share调用完成后，可以在当前线程内访问其他线程(kloop_t)内运行的管道
 * @param channel_ref kchannel_ref_t实例
 * @return kchannel_ref_t实例
 */
FuncExport kchannel_ref_t* knet_channel_ref_share(kchannel_ref_t* channel_ref);

/**
 * 减少管道引用计数，并销毁kchannel_ref_t实例
 * @param channel_ref kchannel_ref_t实例
 */
FuncExport void knet_channel_ref_leave(kchannel_ref_t* channel_ref);

/**
 * 将管道转换为监听管道
 *
 * 由这个监听管道接受的新连接将使用与监听管道相同的发送缓冲区最大数量限制和接受缓冲区长度限制,
 * knet_channel_ref_accept所接受的新连接将被负载均衡，实际运行在哪个kloop_t内依赖于实际运行的情况
 * @param channel_ref kchannel_ref_t实例
 * @param ip IP
 * @param port 端口
 * @param backlog 等待队列上限（listen())
 * @retval error_ok 成功
 * @retval 其他 失败
 */
FuncExport int knet_channel_ref_accept(kchannel_ref_t* channel_ref, const char* ip, int port, int backlog);

/**
 * 主动连接
 *
 * 调用knet_channel_ref_connect的管道会被负载均衡，实际运行在哪个kloop_t内依赖于实际运行的情况
 * @param channel_ref kchannel_ref_t实例
 * @param ip IP
 * @param port 端口
 * @param timeout 连接超时（秒）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
FuncExport int knet_channel_ref_connect(kchannel_ref_t* channel_ref, const char* ip, int port, int timeout);

/**
 * 重新发起连接
 *
 * <pre>
 * 超时的管道将被关闭，建立新管道重连，新管道将使用原有管道的属性，包含回调函数和用户指针
 * 如果timeout设置为0，则使用原有的连接超时，如果timeout>0则使用新的连接超时
 * </pre>
 * @param channel_ref kchannel_ref_t实例
 * @param timeout 连接超时（秒）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
FuncExport int knet_channel_ref_reconnect(kchannel_ref_t* channel_ref, int timeout);

/**
 * 设置管道自动重连
 * <pre>
 * auto_reconnect为非零值则开启自动重连，所有非错误性导致管道关闭，都会自动重连，用户手动调用
 * knet_channel_ref_close将不会触发自动重连
 * </pre>
 * @param channel_ref kchannel_ref_t实例
 * @param auto_reconnect 自动重连标志
 */
FuncExport void knet_channel_ref_set_auto_reconnect(kchannel_ref_t* channel_ref, int auto_reconnect);

/**
 * 检查管道是否开启了自动重连
 * @param channel_ref kchannel_ref_t实例
 * @retval 0 未开启
 * @retval 非零 开启
 */
FuncExport int knet_channel_ref_check_auto_reconnect(kchannel_ref_t* channel_ref);

/**
 * 检测管道是否是通过负载均衡关联到当前的kloop_t
 * @param channel_ref kchannel_ref_t实例
 * @retval 0 不是
 * @retval 非0 是
 */
FuncExport int knet_channel_ref_check_balance(kchannel_ref_t* channel_ref);

/**
 * 检测管道当前状态
 * @param channel_ref kchannel_ref_t实例
 * @param state 需要测试的状态
 * @retval 1 是
 * @retval 0 不是
 */
FuncExport int knet_channel_ref_check_state(kchannel_ref_t* channel_ref, knet_channel_state_e state);

/**
 * 关闭管道
 * @param channel_ref kchannel_ref_t实例
 */
FuncExport void knet_channel_ref_close(kchannel_ref_t* channel_ref);

/**
 * 检查管道是否已经关闭
 * @param channel_ref kchannel_ref_t实例
 * @retval 0 未关闭
 * @retval 非零 关闭
 */
FuncExport int knet_channel_ref_check_close(kchannel_ref_t* channel_ref);

/**
 * 取得管道套接字
 * @param channel_ref kchannel_ref_t实例
 * @return 套接字
 */
FuncExport socket_t knet_channel_ref_get_socket_fd(kchannel_ref_t* channel_ref);

/**
 * 取得管道数据流
 * @param channel_ref kchannel_ref_t实例
 * @return kstream_t实例
 */
FuncExport kstream_t* knet_channel_ref_get_stream(kchannel_ref_t* channel_ref);

/**
 * 取得管道所关联的事件循环
 * @param channel_ref kchannel_ref_t实例
 * @return kloop_t实例
 */
FuncExport kloop_t* knet_channel_ref_get_loop(kchannel_ref_t* channel_ref);

/**
 * 设置管道事件回调
 *
 * 事件回调将在关联的kloop_t实例所在线程内被回调
 * @param channel_ref kchannel_ref_t实例
 * @param cb 回调函数
 */
FuncExport void knet_channel_ref_set_cb(kchannel_ref_t* channel_ref, knet_channel_ref_cb_t cb);

/**
 * 设置管道空闲超时
 *
 * 管道空闲超时依赖读操作作为判断，在timeout间隔内未有可读数据既触发超时
 * @param channel_ref kchannel_ref_t实例
 * @param timeout 超时（秒）
 */
FuncExport void knet_channel_ref_set_timeout(kchannel_ref_t* channel_ref, int timeout);

/**
 * 取得对端地址
 * @param channel_ref kchannel_ref_t实例
 * @return kaddress_t实例
 */
FuncExport kaddress_t* knet_channel_ref_get_peer_address(kchannel_ref_t* channel_ref);

/**
 * 取得本地地址
 * @param channel_ref kchannel_ref_t实例
 * @return kaddress_t实例
 */
FuncExport kaddress_t* knet_channel_ref_get_local_address(kchannel_ref_t* channel_ref);

/**
 * 获取管道UUID
 * @param channel_ref kchannel_t实例
 * @return 管道UUID
 */
FuncExport uint64_t knet_channel_ref_get_uuid(kchannel_ref_t* channel_ref);

/**
 * 测试两个管道引用是否指向同一个管道
 * @param a kchannel_t实例
 * @param b kchannel_t实例
 * @retval 0 不同
 * @retval 非零 相同 
 */
FuncExport int knet_channel_ref_equal(kchannel_ref_t* a, kchannel_ref_t* b);

/**
 * 设置用户数据指针
 * @param channel_ref kchannel_t实例
 * @param ptr 用户数据指针
 */
FuncExport void knet_channel_ref_set_ptr(kchannel_ref_t* channel_ref, void* ptr);

/**
 * 获取用户数据指针
 * @param channel_ref kchannel_t实例
 * @return 用户数据指针
 */
FuncExport void* knet_channel_ref_get_ptr(kchannel_ref_t* channel_ref);

/**
 * 递增当前管道引用计数
 * @param channel_ref kchannel_ref_t实例
 * @return 当前引用计数
 */
FuncExport int knet_channel_ref_incref(kchannel_ref_t* channel_ref);

/**
 * 递减当前管道引用计数
 * @param channel_ref kchannel_ref_t实例
 * @return 当前引用计数
 */
FuncExport int knet_channel_ref_decref(kchannel_ref_t* channel_ref);

/**
 * 检测是否是IPV6管道
 * @param channel_ref kchannel_ref_t实例
 * @retvel 0 不是
 * @retval 非0 是
 */
FuncExport int knet_channel_ref_is_ipv6(kchannel_ref_t* channel_ref);

/**
 * 设置reuseport
 * @param channel_ref kchannel_ref_t实例
 * @retvel 0 不是
 * @retval 非0 是
 */
FuncExport int knet_channel_ref_set_reuseport(kchannel_ref_t* channel_ref);

/** @} */

#endif /* CHANNEL_REF_API_H */
