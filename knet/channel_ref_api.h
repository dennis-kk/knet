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

#ifndef CHANNEL_REF_API_H
#define CHANNEL_REF_API_H

#include "config.h"

/*
 * 增加管道引用计数，并创建与管道关联的新的channel_ref_t实例
 * channel_ref_share调用完成后，可以在当前线程内访问其他线程(loop_t)内运行的管道
 * @param channel_ref channel_ref_t实例
 * @return channel_ref_t实例
 */
extern channel_ref_t* channel_ref_share(channel_ref_t* channel_ref);

/*
 * 减少管道引用计数，并销毁channel_ref_t实例
 * @param channel_ref channel_ref_t实例
 */
extern void channel_ref_leave(channel_ref_t* channel_ref);

/*
 * 主动连接
 * 调用channel_ref_connect的管道不会被负载均衡，将在当前loop_t所运行的线程运行
 * @param channel_ref channel_ref_t实例
 * @param ip IP
 * @param port 端口
 * @param timeout 连接超时（秒）
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int channel_ref_connect(channel_ref_t* channel_ref, const char* ip, int port, int timeout);

/*
 * 将管道转换为监听管道
 * 由这个监听管道接受的新连接将使用与监听管道相同的发送缓冲区最大数量限制和接受缓冲区长度限制,
 * channel_ref_accept所接受的新连接将被负载均衡，实际运行在哪个loop_t内依赖于实际运行的情况
 * @param channel_ref channel_ref_t实例
 * @param ip IP
 * @param port 端口
 * @param backlog 等待队列上限（listen())
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int channel_ref_accept(channel_ref_t* channel_ref, const char* ip, int port, int backlog);

/*
 * 组合两个管道，当input读到的数据自动写入output, output读到的数据会自动写入input
 * 适合网关自动分发数据包，将网关上的客户端管道与对应服务器管道组合
 * @param channel_ref channel_ref_t实例
 * @param partner channel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int channel_ref_tie(channel_ref_t* channel_ref, channel_ref_t* partner);

/*
 * 解除管道组合
 * @param channel_ref channel_ref_t实例
 * @param partner channel_ref_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int channel_ref_untie(channel_ref_t* channel_ref, channel_ref_t* partner);

/*
 * 检测管道是否是通过负载均衡关联到当前的loop_t
 * @param channel_ref channel_ref_t实例
 * @retval 0 不是
 * @retval 非0 是
 */
extern int channel_ref_check_balance(channel_ref_t* channel_ref);

/*
 * 检测管道当前状态
 * @param channel_ref channel_ref_t实例
 * @param state 需要测试的状态
 * @retval 1 是
 * @retval 0 不是
 */
extern int channel_ref_check_state(channel_ref_t* channel_ref, channel_state_e state);

/*
 * 关闭管道
 * @param channel_ref channel_ref_t实例
 */
extern void channel_ref_close(channel_ref_t* channel_ref);

/*
 * 取得管道套接字
 * @param channel_ref channel_ref_t实例
 * @return 套接字
 */
extern socket_t channel_ref_get_socket_fd(channel_ref_t* channel_ref);

/*
 * 取得管道数据流
 * @param channel_ref channel_ref_t实例
 * @return stream_t实例
 */
extern stream_t* channel_ref_get_stream(channel_ref_t* channel_ref);

/*
 * 取得管道所关联的事件循环
 * @param channel_ref channel_ref_t实例
 * @return loop_t实例
 */
extern loop_t* channel_ref_get_loop(channel_ref_t* channel_ref);

/*
 * 设置管道事件回调
 * 事件回调将在关联的loop_t实例所在线程内被回调
 * @param channel_ref channel_ref_t实例
 * @paramn cb 回调函数
 */
extern void channel_ref_set_cb(channel_ref_t* channel_ref, channel_ref_cb_t cb);

/*
 * 设置管道空闲超时
 * 管道空闲超时依赖读操作作为判断，在timeout间隔内未有可读数据既触发超时
 * @param channel_ref channel_ref_t实例
 * @param timeout 超时（秒）
 */
extern void channel_ref_set_timeout(channel_ref_t* channel_ref, int timeout);

/*
 * 取得对端地址
 * @param channel_ref channel_ref_t实例
 * @return address_t实例
 */
extern address_t* channel_ref_get_peer_address(channel_ref_t* channel_ref);

/*
 * 取得本地地址
 * @param channel_ref channel_ref_t实例
 * @return address_t实例
 */
extern address_t* channel_ref_get_local_address(channel_ref_t* channel_ref);

#endif /* CHANNEL_REF_API_H */
