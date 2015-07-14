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

#ifndef LOOP_PROFILE_API_H
#define LOOP_PROFILE_API_H

#include "config.h"

/**
 * 取得已经建立连接的管道数量
 * @param profile loop_profile_t实例
 * @return 建立连接的管道数量
 */
extern uint32_t loop_profile_get_established_channel_count(loop_profile_t* profile);

/**
 * 取得已经建立但还未连接的管道数量
 * @param profile loop_profile_t实例
 * @return 建立但还未连接的管道数量
 */
extern uint32_t loop_profile_get_active_channel_count(loop_profile_t* profile);

/**
 * 取得已经关闭的管道数量
 * @param profile loop_profile_t实例
 * @return 已经关闭的管道数量
 */
extern uint32_t loop_profile_get_close_channel_count(loop_profile_t* profile);

/**
 * 取得已经发送的字节数
 * @param profile loop_profile_t实例
 * @return 已经发送的字节数
 */
extern uint64_t loop_profile_get_sent_bytes(loop_profile_t* profile);

/**
 * 取得已经接收的字节数
 * @param profile loop_profile_t实例
 * @return 已经接收的字节数
 */
extern uint64_t loop_profile_get_recv_bytes(loop_profile_t* profile);

/**
 * 取得发送带宽
 * @param profile loop_profile_t实例
 * @return 带宽(字节/秒)
 */
extern uint32_t loop_profile_get_sent_bandwidth(loop_profile_t* profile);

/**
 * 取得接收带宽
 * @param profile loop_profile_t实例
 * @return 带宽(字节/秒)
 */
extern uint32_t loop_profile_get_recv_bandwidth(loop_profile_t* profile);

/**
 * 将统计信息写入文件
 * @param profile loop_profile_t实例
 * @param fp FILE指针
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int loop_profile_dump_file(loop_profile_t* profile, FILE* fp);

/**
 * 将统计信息写入管道流
 * @param profile loop_profile_t实例
 * @param stream stream_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int loop_profile_dump_stream(loop_profile_t* profile, stream_t* stream);

/**
 * 将统计信息打印到标准输出
 * @param profile loop_profile_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int loop_profile_dump_stdout(loop_profile_t* profile);

#endif /* LOOP_PROFILE_API_H */
