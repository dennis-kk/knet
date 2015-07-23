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

#ifndef RPC_API_H
#define RPC_API_H

/**
 * RPC框架提供了一套底层RPC序列化及反序列化的基础，开发者不应该直接调用这些API，应使用代码生成工具
 * 提供的代码框架来进行RPC操作，生成的代码框架屏蔽了所有烦人的细节，使用起来也会比直接调用这些API更
 * 方便.
 */

#include "config.h"

/**
 * 建立RPC
 * @return krpc_t实例
 */
extern krpc_t* krpc_create();

/**
 * 销毁RPC
 * @param rpc krpc_t实例
 */
extern void krpc_destroy(krpc_t* rpc);

/**
 * 注册RPC调用回调函数
 * @param rpc krpc_t实例
 * @param rpcid 回调ID
 * @param cb 回调函数指针
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_add_cb(krpc_t* rpc, uint16_t rpcid, krpc_cb_t cb);

/**
 * 删除注册过的RPC调用回调函数
 * @param rpc krpc_t实例
 * @param rpcid 回调ID
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_del_cb(krpc_t* rpc, uint16_t rpcid);

/**
 * 取得注册过的RPC调用回调函数
 * @param rpc krpc_t实例
 * @param rpcid 回调ID
 * @return 回调函数指针
 */
extern krpc_cb_t krpc_get_cb(krpc_t* rpc, uint16_t rpcid);

/**
 * 处理RPC调用，从数据流反序列化RPC调用，并调用回调函数
 * @param rpc krpc_t实例
 * @param stream 数据流
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_proc(krpc_t* rpc, kstream_t* stream);

/**
 * 发起RPC调用
 * @param rpc krpc_t实例
 * @param stream 数据流
 * @param rpcid 回调ID
 * @param o 参数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_call(krpc_t* rpc, kstream_t* stream, uint16_t rpcid, krpc_object_t* o);

/**
 * 设置签名加密回调
 * @param rpc krpc_t实例
 * @param func 回调函数指针
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_set_encrypt_cb(krpc_t* rpc, krpc_encrypt_t func);

/**
 * 设置签名解密回调
 * @param rpc krpc_t实例
 * @param func 回调函数指针
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_set_decrypt_cb(krpc_t* rpc, krpc_decrypt_t func);

#endif /* RPC_API_H */
