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

#ifndef BROADCAST_API_H
#define BROADCAST_API_H

#include "config.h"

/**
 * @defgroup broadcast 广播
 * 广播域
 *
 * <pre>
 * 管道可以加入广播域，加入后通过knet_broadcast_write方法可以发送数据到所有已经加入域内的管道.
 * 调用knet_broadcast_create创建一个广播域，knet_broadcast_destroy销毁广播域.
 *
 * knet_broadcast_join加入一个广播域，knet_broadcast_join函数返回时创建了一个新的管道引用.
 *
 * 调用knet_broadcast_leave时应该使用这个返回值而不是使用knet_broadcast_join的第二个参数，新创建
 * 的管道引用会提升管道的引用计数，如果你关闭了这个管道你可以通过调用knet_broadcast_leave减少管
 * 道的引用计数，从而可以让kloop_t真正销毁管道.
 *
 * 调用knet_broadcast_get_count可以得知广播域内的管道引用数量，调用knet_broadcast_write发起一个
 * 广播操作，所有域内管道都会收到你广播的数据.
 * </pre>
 * @{
 */

/**
 * 创建广播域
 * @return kbroadcast_t实例
 */
extern kbroadcast_t* knet_broadcast_create();

/**
 * 销毁广播域
 *
 * 销毁的同时会将所有还在域内的管道引用销毁
 * @param broadcast kbroadcast_t实例
 */
extern void knet_broadcast_destroy(kbroadcast_t* broadcast);

/**
 * 加入广播域
 *
 * 加入成功会生成一个新的引用
 * @param broadcast kbroadcast_t实例
 * @param channel_ref kchannel_ref_t
 * @return kchannel_ref_t实例，新的在广播域内的引用
 */
extern kchannel_ref_t* knet_broadcast_join(kbroadcast_t* broadcast, kchannel_ref_t* channel_ref);

/**
 * 离开广播域
 *
 * 函数返回后管道引用已经被销毁，不要尝试再次访问这个引用
 * @param broadcast kbroadcast_t实例
 * @param channel_ref kchannel_ref_t实例，由knet_broadcast_join()返回
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_broadcast_leave(kbroadcast_t* broadcast, kchannel_ref_t* channel_ref);

/**
 * 取得广播域内管道数量
 * @param broadcast kbroadcast_t实例
 * @return 管道数量
 */
extern int knet_broadcast_get_count(kbroadcast_t* broadcast);

/**
 * 广播
 * @param broadcast kbroadcast_t实例
 * @param buffer 缓冲区指针
 * @param size 缓冲区长度
 * @return 发送成功管道的数量
 */
extern int knet_broadcast_write(kbroadcast_t* broadcast, char* buffer, uint32_t size);

/** @} */

#endif /* BROADCAST_API_H */
