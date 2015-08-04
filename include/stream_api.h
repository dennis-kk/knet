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

#ifndef STREAM_API_H
#define STREAM_API_H

#include "config.h"

/**
 * @defgroup stream 流
 * 管道流
 *
 * <pre>
 * 管道流
 *
 * kstream_t通过调用函数knet_channel_ref_get_stream取得. 管道流提供了基于流的数据操作
 * 以及特殊的针对性的方法用于提高操作效率.
 * 
 * 1. knet_stream_available   获取流内可读字节数
 * 2. knet_stream_eat_all     丢弃流内所有可读字节
 * 3. knet_stream_eat         丢弃流内指定数量的字节
 * 4. knet_stream_pop         从流内读取数据
 * 5. knet_stream_push        向流内写数据
 * 6. knet_stream_copy        从流内拷贝指定数量的可读字节，但不清除这些字节，通常用于协议检测
 * 7. knet_stream_push_stream 将流内所有可读字节写入另一个流，不需要额外拷贝, 可用于网关的数据中转
 * 8. knet_stream_copy_stream 将流内所有可读字节写入另一个流，不需要额外拷贝, 但不清除这些字节，可用于广播
 *
 * 以上这些函数的设计除了基础的流本身的功能以外，还考虑了特定领域的应用，同时兼顾了效率.
 *
 * </pre>
 * @{
 */

/**
 * 取得数据流内可读字节数
 * @param stream kstream_t实例
 * @return 可读字节数
 */
extern int knet_stream_available(kstream_t* stream);

/**
 * 清空数据流
 * @param stream kstream_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_eat_all(kstream_t* stream);

/**
 * 删除指定长度数据
 * @param stream kstream_t实例
 * @param size 需要删除的长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_eat(kstream_t* stream, int size);

/**
 * 从数据流内读取数据并清除数据
 * @param stream kstream_t实例
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_pop(kstream_t* stream, void* buffer, int size);

/**
 * 从数据流内查找指定的结束符，并取出遍历过的数据（包含结束符）
 * @param stream kstream_t实例
 * @param end 结束符
 * @param buffer 缓冲区
 * @param size 缓冲区大小，返回实际的读取的字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_pop_until(kstream_t* stream, const char* end, void* buffer, int* size);

/**
 * 向数据流内写数据
 * @param stream kstream_t实例
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_push(kstream_t* stream, const void* buffer, int size);

/**
 * 向数据流写数据，可变参数字符串
 *
 * 一次写入的长度不能超过1024
 * @param stream kstream_t实例
 * @param format 字符串格式
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_push_varg(kstream_t* stream, const char* format, ...);

/**
 * 从数据流内拷贝数据，但不清除数据流内数据
 * @param stream kstream_t实例
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_copy(kstream_t* stream, void* buffer, int size);

/**
 * 将stream内数据写入target, 并清除stream内数据
 * @param stream kstream_t实例
 * @param target kstream_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_push_stream(kstream_t* stream, kstream_t* target);

/**
 * 将stream内数据写入target, 但不清除stream内数据
 * @param stream kstream_t实例
 * @param target kstream_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_stream_copy_stream(kstream_t* stream, kstream_t* target);

/**
 * 获取流所属的管道引用
 * @param stream kstream_t实例
 * @return kchannel_ref_t实例
 */
extern kchannel_ref_t* knet_stream_get_channel_ref(kstream_t* stream);

/** @} */

#endif /* STREAM_API_H */
