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

#ifndef HASH_H
#define HASH_H

#include "config.h"

/*
 * 内部使用，哈希表，同时支持数字或字符串作为key
 */

/*
 * 建立哈希表
 * @param size 哈希表桶数量, 0将使用默认桶数量
 * @param dtor 用户自定义值销毁函数
 * @return hash_t实例
 */
hash_t* hash_create(uint32_t size, hash_dtor_t dtor);

/*
 * 销毁哈希表
 * @param hash hash_t实例
 */
void hash_destroy(hash_t* hash);

/*
 * 添加元素
 * @param hash hash_t实例
 * @param key 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_add(hash_t* hash, uint32_t key, void* value);

/*
 * 添加元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_add_string_key(hash_t* hash, const char* key, void* value);

/*
 * 移除元素
 * @param hash hash_t实例
 * @param key 键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
void* hash_remove(hash_t* hash, uint32_t key);

/*
 * 移除元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
void* hash_remove_string_key(hash_t* hash, const char* key);

/*
 * 销毁元素
 * @param hash hash_t实例
 * @param key 键
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_delete(hash_t* hash, uint32_t key);

/*
 * 销毁元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int hash_delete_string_key(hash_t* hash, const char* key);

/*
 * 获取元素
 * @param hash hash_t实例
 * @param key 键
 * @retval 0 未找到
 * @retval 有效指针
 */
void* hash_get(hash_t* hash, uint32_t key);

/*
 * 获取元素
 * @param hash hash_t实例
 * @param key 字符串键
 * @retval 0 未找到
 * @retval 有效指针
 */
void* hash_get_string_key(hash_t* hash, const char* key);

/*
 * 取得元素数量
 * @param hash hash_t实例
 * @return 元素数量
 */
uint32_t hash_get_size(hash_t* hash);

/*
 * 重置遍历器，取第一个元素
 * @param hash hash_t实例
 * @retval 0 没有元素
 * @retval hash_value_t实例
 */
hash_value_t* hash_get_first(hash_t* hash);

/*
 * 哈希表遍历器的下一个元素
 * @param hash hash_t实例
 * @retval 0 没有元素
 * @retval hash_value_t实例
 */
hash_value_t* hash_next(hash_t* hash);

/* 遍历哈希表，可以在遍历过程中删除或销毁当前元素 */
#define hash_for_each_safe(hash, value) \
    for (value = hash_get_first(hash); (value); value = hash_get_next(hash))

#endif /* HASH_H */
