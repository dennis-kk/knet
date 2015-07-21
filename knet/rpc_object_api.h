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

#ifndef RPC_OBJECT_API_H
#define RPC_OBJECT_API_H

/**
 * @defgroup rpc 远程调用
 * 远程调用
 *
 * <pre>
 * 轻量级RPC框架
 *
 * krpc_object_t作为一个泛型的对象抽象接口，可以作为对象系统内任何类型的对象存在，
 * 在设置方法调用后会确定对象的类型，但一旦确定是不能更改，krpc_object_t提供了统一的
 * 接口来进行marshal/unmarshal(列集/散集)操作：
 *
 * 1. krpc_object_marshal          marshal并发送到管道流
 * 2. krpc_object_marshal_buffer   marshal到缓冲区
 * 3. krpc_object_unmarshal        从管道流接收并unmarshal
 * 4. krpc_object_unmarshal_buffer 从缓冲区unmarshal
 *
 * 以上两类方法实现了从数据流到对象及对象到数据流的操作，作为RPC框架的基础.
 *
 * 对象类型系统包含如下类型：
 *
 * 1. 数字       整数，浮点数
 * 2. 字符串     字符串
 * 3. 数组(向量) 数组类容器
 * 4. 表(字典)   字典类容器
 *
 * 操作以上类型的方法都明确的使用了前缀：
 * 
 * 1. krpc_number_ 数字
 * 2. krpc_string_ 字符串
 * 3. krpc_vector_ 数组
 * 4. krpc_map_    表
 *
 * 容器类对象(数组，表)内可以容纳其他任意类型的对象(包含和自己相同类型对象),
 * 一旦使用set类或clear类方法后，对象的类型就确定，譬如：调用krpc_set_number_i8
 * 后， 对象的类型为数字，8位有符号整数. 类型操作函数没有提供非常丰富的方法来操作对象，
 * 因为整套框架本身是用于RPC并非通用的容器库，使用的定位与传统的容器库有本质的不同.
 *
 * </pre>
 * @{
 */

/* TODO 待测试 */

#include "config.h"

/**
 * 所有set类方法调用时确定krpc_object_t实际类型，类型一旦确定不能再次更改
 */

/**
 * 建立RPC对象
 * @return krpc_object_t实例
 */
extern krpc_object_t* krpc_object_create();

/**
 * 销毁RPC对象
 * @param o krpc_object_t实例
 */
extern void krpc_object_destroy(krpc_object_t* o);

/**
 * 测试RPC对象类型
 * @param o krpc_object_t实例
 * @param type 类型
 * @retval 0 不是type类型
 * @retval 非零 是type类型
 */
extern int krpc_object_check_type(krpc_object_t* o, knet_rpc_type_e type);

/**
 * 取得序列化后的长度
 * @param o krpc_object_t实例
 * @return 取得序列化后的长度
 */
extern uint16_t krpc_object_get_marshal_size(krpc_object_t* o);

/**
 * 序列化到数据流
 * @param o krpc_object_t实例
 * @param stream kstream_t实例
 * @param bytes 写入流的数据字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_object_marshal(krpc_object_t* o, kstream_t* stream, uint16_t* bytes);

/**
 * 序列化到缓冲区
 * @param o krpc_object_t实例
 * @param buffer 缓冲区指针
 * @param length 缓冲区长度
 * @param bytes 写入缓冲区的数据字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_object_marshal_buffer(krpc_object_t* o, char* buffer, uint16_t length, uint16_t* bytes);

/**
 * 从数据流反序列化对象
 * @param stream kstream_t实例
 * @param o 存储得到的对象指针
 * @param bytes 读取的流数据字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_object_unmarshal(kstream_t* stream, krpc_object_t** o, uint16_t* bytes);

/**
 * 从缓冲区反序列化对象
 * @param buffer 缓冲区指针
 * @param size 缓冲区长度
 * @param o 存储得到的对象指针
 * @param bytes 消耗的字节数
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_object_unmarshal_buffer(char* buffer, uint16_t size, krpc_object_t** o, uint16_t* bytes);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param i8 有符号8位
 */
extern void krpc_number_set_i8(krpc_object_t* o, int8_t i8);

/**
 * 获取有符号8位
 * @param o krpc_object_t实例
 * @return 有符号8位
 */
extern int8_t krpc_number_get_i8(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param i16 有符号16位
 */
extern void krpc_number_set_i16(krpc_object_t* o, int16_t i16);

/**
 * 获取有符号16位
 * @param o krpc_object_t实例
 * @return 有符号16位
 */
extern int16_t krpc_number_get_i16(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param i32 有符号32位
 */
extern void krpc_number_set_i32(krpc_object_t* o, int32_t i32);

/**
 * 获取有符号32位
 * @param o krpc_object_t实例
 * @return 有符号32位
 */
extern int32_t krpc_number_get_i32(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param i64 有符号64位
 */
extern void krpc_number_set_i64(krpc_object_t* o, int64_t i64);

/**
 * 获取有符号64位
 * @param o krpc_object_t实例
 * @return 有符号64位
 */
extern int64_t krpc_number_get_i64(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param ui8 无符号8位
 */
extern void krpc_number_set_ui8(krpc_object_t* o, uint8_t ui8);

/**
 * 获取无符号8位
 * @param o krpc_object_t实例
 * @return 无符号8位
 */
extern uint8_t krpc_number_get_ui8(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param ui16 无符号16位
 */
extern void krpc_number_set_ui16(krpc_object_t* o, uint16_t ui16);

/**
 * 获取无符号16位
 * @param o krpc_object_t实例
 * @return 无符号16位
 */
extern uint16_t krpc_number_get_ui16(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param ui32 无符号32位
 */
extern void krpc_number_set_ui32(krpc_object_t* o, uint32_t ui32);

/**
 * 获取无符号32位
 * @param o krpc_object_t实例
 * @return 无符号32位
 */
extern uint32_t krpc_number_get_ui32(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param ui64 无符号64位
 */
extern void krpc_number_set_ui64(krpc_object_t* o, int64_t ui64);

/**
 * 获取无符号64位
 * @param o krpc_object_t实例
 * @return 无符号64位
 */
extern uint64_t krpc_number_get_ui64(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param f32 浮点数32位
 */
extern void krpc_number_set_f32(krpc_object_t* o, float32_t f32);

/**
 * 获取浮点数32位
 * @param o krpc_object_t实例
 * @return 浮点数32位
 */
extern float32_t krpc_number_get_f32(krpc_object_t* o);

/**
 * 设置数字
 * @param o krpc_object_t实例
 * @param f64 浮点数64位
 */
extern void krpc_number_set_f64(krpc_object_t* o, float64_t f64);

/**
 * 获取浮点数64位
 * @param o krpc_object_t实例
 * @return 浮点数64位
 */
extern float64_t krpc_number_get_f64(krpc_object_t* o);

/**
 * 设置字符串
 * @param o krpc_object_t实例
 * @param s 字符串指针
 */
extern void krpc_string_set(krpc_object_t* o, const char* s);

/**
 * 设置字符串
 * @param o krpc_object_t实例
 * @param size 字符串长度，包含结尾符
 * @param s 字符串指针
 */
extern void krpc_string_set_s(krpc_object_t* o, const char* s, uint16_t size);

/**
 * 获取字符串指针
 * @param o krpc_object_t实例
 * @return 字符串指针
 */
extern const char* krpc_string_get(krpc_object_t* o);

/**
 * 获取字符串长度，包含结尾符
 * @param o krpc_object_t实例
 * @return 字符串长度
 */
extern uint16_t krpc_string_get_size(krpc_object_t* o);

/**
 * 添加到数组尾，数组默认长度为8，每次递增8
 * @param v krpc_object_t实例
 * @param o 新增的数组元素
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_vector_push_back(krpc_object_t* v, krpc_object_t* o);

/**
 * 取得数组元素个数
 * @param v krpc_object_t实例
 * @return 数组元素个数
 */
extern uint32_t krpc_vector_get_size(krpc_object_t* v);

/**
 * 按索引取得数组元素
 * @param v krpc_object_t实例
 * @param index 下标索引
 * @return 数组元素
 */
extern krpc_object_t* krpc_vector_get(krpc_object_t* v, int index);

/**
 * 按索引设置数组元素，若已存在元素则销毁旧元素
 * @param v krpc_object_t实例
 * @param o 新元素
 * @param index 下标索引
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_vector_set(krpc_object_t* v, krpc_object_t* o, int index);

/**
 * 清空数组
 * @param v krpc_object_t实例
 */
extern void krpc_vector_clear(krpc_object_t* v);

/**
 * 添加<k, v>
 * @param m krpc_object_t实例
 * @param k key
 * @param v value
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_map_insert(krpc_object_t* m, krpc_object_t* k, krpc_object_t* v);

/**
 * 更新<k, v>，如果k不存在则插入
 * @param m krpc_object_t实例
 * @param k key
 * @param v value
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int krpc_map_update(krpc_object_t* m, krpc_object_t* k, krpc_object_t* v);

/**
 * 查找
 * @param m krpc_object_t实例
 * @param k key
 * @retval krpc_object_t实例
 * @retval 0 未找到
 */
extern krpc_object_t* krpc_map_get(krpc_object_t* m, krpc_object_t* k);

/**
 * 取得表<kev, value>个数
 * @param m krpc_object_t实例
 * @return <kev, value>个数
 */
extern uint32_t krpc_map_get_size(krpc_object_t* m);

/**
 * 重置遍历器并取第一个<key, value>
 * @param m krpc_object_t实例
 * @param k key
 * @param v value
 * @retval 0 没有
 * @retval 非零 有
 */
extern int krpc_map_get_first(krpc_object_t* m, krpc_object_t** k, krpc_object_t** v);

/**
 * 取下一个未遍历的<key, value>
 * @param m krpc_object_t实例
 * @param k key
 * @param v value
 * @retval 0 没有
 * @retval 非零 有
 */
extern int krpc_map_next(krpc_object_t* m, krpc_object_t** k, krpc_object_t** v);

/**
 * 清空表
 * @param m krpc_object_t实例
 */
extern void krpc_map_clear(krpc_object_t* m);

/** @} */

#endif /* RPC_OBJECT_API_H */
