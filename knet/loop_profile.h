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

#ifndef LOOP_PROFILE_H
#define LOOP_PROFILE_H

#include "config.h"
#include "loop_profile_api.h"

/**
 * 建立kloop_t统计器
 * @param loop kloop_t实例
 * @return kloop_profile_t实例
 */
kloop_profile_t* knet_loop_profile_create(kloop_t* loop);

/**
 * 销毁统计器
 * @param profile kloop_profile_t实例
 */
void knet_loop_profile_destroy(kloop_profile_t* profile);

/**
 * 增加已建立连接的管道数量
 * @param profile kloop_profile_t实例
 * @return 已建立连接的管道数量
 */
uint32_t knet_loop_profile_increase_established_channel_count(kloop_profile_t* profile);

/**
 * 减少已建立连接的管道数量
 * @param profile kloop_profile_t实例
 * @return 已建立连接的管道数量
 */
uint32_t knet_loop_profile_decrease_established_channel_count(kloop_profile_t* profile);

/**
 * 增加已建立但未连接的管道数量
 * @param profile kloop_profile_t实例
 * @return 已建立但未连接的管道数量
 */
uint32_t knet_loop_profile_increase_active_channel_count(kloop_profile_t* profile);

/**
 * 减少已建立但未连接的管道数量
 * @param profile kloop_profile_t实例
 * @return 已建立但未连接的管道数量
 */
uint32_t knet_loop_profile_decrease_active_channel_count(kloop_profile_t* profile);

/**
 * 增加已关闭管道数量
 * @param profile kloop_profile_t实例
 * @return 已关闭管道数量
 */
uint32_t knet_loop_profile_increase_close_channel_count(kloop_profile_t* profile);

/**
 * 减少已关闭管道数量
 * @param profile kloop_profile_t实例
 * @return 已关闭管道数量
 */
uint32_t knet_loop_profile_decrease_close_channel_count(kloop_profile_t* profile);

/**
 * 增加发送字节数
 * @param profile kloop_profile_t实例
 * @param send_bytes 已发送的字节数
 * @return 发送字节数
 */
uint64_t knet_loop_profile_add_send_bytes(kloop_profile_t* profile, uint64_t send_bytes);

/**
 * 增加接收字节数
 * @param profile kloop_profile_t实例
 * @param recv_bytes 已接收的字节数
 * @return 接收字节数
 */
uint64_t knet_loop_profile_add_recv_bytes(kloop_profile_t* profile, uint64_t recv_bytes);

#endif /* LOOP_PROFILE_H */
