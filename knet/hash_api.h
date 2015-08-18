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

#ifndef HASH_API_H
#define HASH_API_H

#include "config.h"

/*
 * 哈希表，同时支持数字或字符串作为key
 */

/**
 * 取得自定义值
 * @param hash_value khash_value_t实例
 * @return 自定义值
 */
extern void* hash_value_get_value(khash_value_t* hash_value);

/**
 * 取得数字键
 * @param hash_value khash_value_t实例
 * @return 数字键
 */
extern uint32_t hash_value_get_key(khash_value_t* hash_value);

/**
 * 取得字符串键
 * @param hash_value khash_value_t实例
 * @return 字符串键
 */
extern const char* hash_value_get_string_key(khash_value_t* hash_value);

/**
 * 建立哈希表
 * @param size 哈希表桶数量, 0将使用默认桶数量
 * @param dtor 用户自定义值销毁函数
 * @return khash_t实例
 */
extern khash_t* hash_create(uint32_t size, knet_hash_dtor_t dtor);

/**
 * 销毁哈希表
 * @param hash khash_t实例
 */
extern void hash_destroy(khash_t* hash);

/**
 * 添加元素
 * @param hash khash_t实例
 * @param key 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int hash_add(khash_t* hash, uint32_t key, void* value);

/**
 * 添加元素
 * @param hash khash_t实例
 * @param key 字符串键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int hash_add_string_key(khash_t* hash, const char* key, void* value);

/**
 * 移除元素
 * @param hash khash_t实例
 * @param key 键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
extern void* hash_remove(khash_t* hash, uint32_t key);

/**
 * 移除元素
 * @param hash khash_t实例
 * @param key 字符串键
 * @retval 0 未找到
 * @retval 有效指针 值
 */
extern void* hash_remove_string_key(khash_t* hash, const char* key);

/**
 * 销毁元素
 * @param hash khash_t实例
 * @param key 键
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int hash_delete(khash_t* hash, uint32_t key);

/**
 * 替换
 * @param hash khash_t实例
 * @param key 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int hash_replace(khash_t* hash, uint32_t key, void* value);

/**
 * 替换
 * @param hash khash_t实例
 * @param key 字符串键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int hash_replace_string_key(khash_t* hash, const char* key, void* value);

/**
 * 销毁元素
 * @param hash khash_t实例
 * @param key 字符串键
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int hash_delete_string_key(khash_t* hash, const char* key);

/**
 * 获取元素
 * @param hash khash_t实例
 * @param key 键
 * @retval 0 未找到
 * @retval 有效指针
 */
extern void* hash_get(khash_t* hash, uint32_t key);

/**
 * 获取元素
 * @param hash khash_t实例
 * @param key 字符串键
 * @retval 0 未找到
 * @retval 有效指针
 */
extern void* hash_get_string_key(khash_t* hash, const char* key);

/**
 * 取得元素数量
 * @param hash khash_t实例
 * @return 元素数量
 */
extern uint32_t hash_get_size(khash_t* hash);

/**
 * 重置遍历器，取第一个元素
 * @param hash khash_t实例
 * @retval 0 没有元素
 * @retval khash_value_t实例
 */
extern khash_value_t* hash_get_first(khash_t* hash);

/**
 * 哈希表遍历器的下一个元素
 * @param hash khash_t实例
 * @retval 0 没有元素
 * @retval khash_value_t实例
 */
extern khash_value_t* hash_next(khash_t* hash);

/* 遍历哈希表，可以在遍历过程中删除或销毁当前元素, 遍历宏不是线程安全的 */
#define hash_for_each_safe(hash, value) \
    for (value = hash_get_first(hash); (value); value = hash_next(hash))

#endif /* HASH_API_H */
