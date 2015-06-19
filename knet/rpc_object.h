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

#ifndef RPC_OBJECT_H
#define RPC_OBJECT_H

/* TODO 待测试 */

#include "config.h"
#include "rpc_object_api.h"

/*
 * 取得序列化后的长度
 * @param o krpc_object_t实例
 * @return 取得序列化后的长度
 */
uint16_t krpc_object_get_marshal_size(krpc_object_t* o);

/*
 * 序列化到数据流
 * @param o krpc_object_t实例
 * @param stream stream_t实例
 * @param bytes 写入流的数据字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int krpc_object_marshal(krpc_object_t* o, stream_t* stream, uint16_t* bytes);

/*
 * 从数据流反序列化对象
 * @param stream stream_t实例
 * @param o 存储得到的对象指针
 * @param length 读取的流数据字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int krpc_object_unmarshal(stream_t* stream, krpc_object_t** o, uint16_t* length);

/*
 * 获取数字对象序列化长度
 * @param o krpc_object_t实例
 * @return 数字对象序列化长度
 */
uint16_t krpc_number_get_marshal_size(krpc_object_t* o);

/*
 * 设置字符串长度
 * @param o krpc_object_t实例
 * @param size 长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int krpc_string_set_size(krpc_object_t* o, uint16_t size);

/*
 * 扩展数组
 * @param o krpc_object_t实例
 */
void krpc_vector_enlarge(krpc_object_t* o);

#endif /* RPC_OBJECT_H */
