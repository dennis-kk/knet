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

#ifndef CHANNEL_REF_H
#define CHANNEL_REF_H

#include "config.h"
#include "channel_ref_api.h"

/**
 * 创建管道引用
 * @param loop kloop_t实例
 * @param channel kchannel_t实例
 * @return kchannel_ref_t实例
 */
kchannel_ref_t* knet_channel_ref_create(kloop_t* loop, kchannel_t* channel);

/**
 * 销毁管道引用
 * 管道引用计数为零时才能被实际销毁
 * @param channel_ref kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_ref_destroy(kchannel_ref_t* channel_ref);

/**
 * 关闭管道并检查重连
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_close_check_reconnect(kchannel_ref_t* channel_ref);

/**
 * 写入
 * 管道引用计数为零时才能被实际销毁
 * @param channel_ref kchannel_ref_t实例
 * @param data 写入数据指针
 * @param size 数据长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_ref_write(kchannel_ref_t* channel_ref, const char* data, int size);

/**
 * 为通过accept()返回的套接字创建管道引用
 * @param channel_ref kchannel_ref_t实例
 * @param loop kloop_t实例
 * @param client_fd 通过accept()得到的套接字
 * @param event 是否投递事件和设置管道状态
 * @return kchannel_ref_t实例
 */
kchannel_ref_t* knet_channel_ref_accept_from_socket_fd(kchannel_ref_t* channel_ref, kloop_t* loop, socket_t client_fd, int event);

/**
 * 取得管道相关联的kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 * @return kloop_t实例
 */
kloop_t* knet_channel_ref_choose_loop(kchannel_ref_t* channel_ref);

/**
 * 设置管道链表节点
 * @param channel_ref kchannel_ref_t实例
 * @param node 链表节点
 */
void knet_channel_ref_set_loop_node(kchannel_ref_t* channel_ref, kdlist_node_t* node);

/**
 * 取得管道链表节点
 * @param channel_ref kchannel_ref_t实例
 * @return kdlist_node_t实例
 */
kdlist_node_t* knet_channel_ref_get_loop_node(kchannel_ref_t* channel_ref);

/**
 * 在kloop_t所运行的线程内完成连接流程
 * @param channel_ref kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_ref_connect_in_loop(kchannel_ref_t* channel_ref);

/**
 * 在kloop_t所运行的线程内完成连接流程
 * @param channel_ref kchannel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_channel_ref_connect_in_loop_address(kchannel_ref_t* channel_ref);

/**
 * 在kloop_t所运行的线程内完成接收新连接流程
 * 通过负载均衡触发
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_update_accept_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 在其他完成了listen()和bind()操作，在当前线程的kloop_t内监听
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_accept_async(kchannel_ref_t* channel_ref);

/**
 * 在kloop_t所运行的线程内完成关闭流程
 * 通过跨线程关闭触发
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_update_close_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * 在kloop_t所运行的线程内发送
 * 通过跨线程发送触发
 * @param loop kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 * @param send_buffer kbuffer_t实例
 */
void knet_channel_ref_update_send_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref, kbuffer_t* send_buffer);

/**
 * 设置管道自定义标志
 * @param channel_ref kchannel_ref_t实例
 * @param flag 自定义标志
 */
void knet_channel_ref_set_flag(kchannel_ref_t* channel_ref, int flag);

/**
 * 取得管道自定义标志
 * @param channel_ref kchannel_ref_t实例
 * @return 自定义标志
 */
int knet_channel_ref_get_flag(kchannel_ref_t* channel_ref);

/**
 * 设置管道自定义数据
 * @param channel_ref kchannel_ref_t实例
 * @param data 自定义数据指针
 */
void knet_channel_ref_set_data(kchannel_ref_t* channel_ref, void* data);

/**
 * 取得管道自定义数据
 * @param channel_ref kchannel_ref_t实例
 * @return 自定义数据指针
 */
void* knet_channel_ref_get_data(kchannel_ref_t* channel_ref);

/**
 * 设置管道所属（相关联）kloop_t实例
 * @param channel_ref kchannel_ref_t实例
 * @param loop kloop_t实例
 */
void knet_channel_ref_set_loop(kchannel_ref_t* channel_ref, kloop_t* loop);

/**
 * 投递管道事件
 * @param channel_ref kchannel_ref_t实例
 * @param e 管道事件
 */
void knet_channel_ref_set_event(kchannel_ref_t* channel_ref, knet_channel_event_e e);

/**
 * 获取管道已经投递的事件集合
 * @param channel_ref kchannel_ref_t实例
 * @return 管道事件集合
 */
knet_channel_event_e knet_channel_ref_get_event(kchannel_ref_t* channel_ref);

/**
 * 取消管道事件
 * @param channel_ref kchannel_ref_t实例
 * @param e 管道事件
 */
void knet_channel_ref_clear_event(kchannel_ref_t* channel_ref, knet_channel_event_e e);

/**
 * 检测是否投递了事件
 * @param channel_ref kchannel_ref_t实例
 * @param event 管道事件
 * @retval 0 没有投递
 * @retval 非零 已经投递
 */
int knet_channel_ref_check_event(kchannel_ref_t* channel_ref, knet_channel_event_e event);

/**
 * 设置管道状态
 * @param channel_ref kchannel_ref_t实例
 * @param state 管道状态
 */
void knet_channel_ref_set_state(kchannel_ref_t* channel_ref, knet_channel_state_e state);

/**
 * 管道事件通知
 * @param channel_ref kchannel_ref_t实例
 * @param e 管道事件
 * @param ts 当前时间戳（秒）
 */
void knet_channel_ref_update(kchannel_ref_t* channel_ref, knet_channel_event_e e, time_t ts);

/**
 * 管道事件处理-监听到新连接请求
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_update_accept(kchannel_ref_t* channel_ref);

/**
 * 管道事件处理-发起连接完成
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_update_connect(kchannel_ref_t* channel_ref);

/**
 * 管道事件处理-有数据可读
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_update_recv(kchannel_ref_t* channel_ref);

/**
 * 管道事件处理-可以发送数据
 * @param channel_ref kchannel_ref_t实例
 */
void knet_channel_ref_update_send(kchannel_ref_t* channel_ref);

/**
 * 检测管道发起的连接操作是否超时
 * @param channel_ref kchannel_ref_t实例
 * @param ts 当前时间戳（秒）
 * @retval 0 没有超时
 * @retval 非零 超时
 */
int knet_channel_ref_check_connect_timeout(kchannel_ref_t* channel_ref, time_t ts);

/**
 * 检测管道是否空闲超时
 * @param channel_ref kchannel_ref_t实例
 * @param ts 当前时间戳（秒）
 * @retval 0 没有超时
 * @retval 非零 超时
 */
int knet_channel_ref_check_timeout(kchannel_ref_t* channel_ref, time_t ts);

/**
 * 取得管道读缓冲区
 * @param channel_ref kchannel_ref_t实例
 * @return kringbuffer_t实例
 */
kringbuffer_t* knet_channel_ref_get_ringbuffer(kchannel_ref_t* channel_ref);

/**
 * 取得管道事件回调
 * @param channel_ref kchannel_ref_t实例
 * @return 回调函数指针
 */
knet_channel_ref_cb_t knet_channel_ref_get_cb(kchannel_ref_t* channel_ref);

/**
 * 设置域链表节点
 * @param channel_ref kchannel_ref_t实例
 * @param node 域链表节点
 */
void knet_channel_ref_set_domain_node(kchannel_ref_t* channel_ref, kdlist_node_t* node);

/**
 * 取得域链表节点
 * @param channel_ref kchannel_ref_t实例
 * @return kdlist_node_t实例
 */
kdlist_node_t* knet_channel_ref_get_domain_node(kchannel_ref_t* channel_ref);

/**
 * 检查管道引用是否通过调用knet_channel_ref_share()建立
 * @param channel_ref kchannel_ref_t实例
 * @retval 0 不是
 * @retval 非零 是
 */
int knet_channel_ref_check_share(kchannel_ref_t* channel_ref);

/**
 * 设置域ID
 * @param channel_ref kchannel_ref_t实例
 * @param domain_id 域ID
 */
void knet_channel_ref_set_domain_id(kchannel_ref_t* channel_ref, uint64_t domain_id);

/**
 * 取得域ID
 * @param channel_ref kchannel_ref_t实例
 * @return 域ID
 */
uint64_t knet_channel_ref_get_domain_id(kchannel_ref_t* channel_ref);

/**
 * 测试管道引用是否为零
 * @param channel_ref kchannel_ref_t实例
 * @return 0 不为零
 * @return 非零 为零
 */
int knet_channel_ref_check_ref_zero(kchannel_ref_t* channel_ref);

/**
 * 获取管道引用计数
 * @param channel_ref kchannel_ref_t实例
 * @return 管道引用计数
 */
int knet_channel_ref_get_ref(kchannel_ref_t* channel_ref);

/**
 * 设置管道用户数据
 * @param channel_ref kchannel_ref_t实例
 * @param data 用户数据指针
 */
void knet_channel_ref_set_user_data(kchannel_ref_t* channel_ref, void* data);

/**
 * 获取管道用户数据
 * @param channel_ref kchannel_ref_t实例
 * @return 用户数据指针
 */
void* knet_channel_ref_get_user_data(kchannel_ref_t* channel_ref);

#endif /* CHANNEL_REF_H */
