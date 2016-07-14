/*
 * Copyright (c) 2014-2016, dennis wang
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

#ifndef TRIE_API_H
#define TRIE_API_H

#include "config.h"

/**
 * @defgroup trie 字符串KV树
 * 字符串树
 * <pre>
 * 提供一个三路树结构用于快速查找字符串，三路树分别为{left, center, right}，
 * 满足left < center < right，查找，删除，插入效率为O(n)，n为字符串长度.
 * 字符串树使用字符串作为键，void*类型作为值，也可以作为哈希表使用，用户可以提供
 * 一个值销毁函数用于trie树销毁时的自定义值清理回调.
 * </pre>
 * @{
 */

/**
 * 建立trie
 * @return ktrie_t实例
 */
extern ktrie_t* trie_create();

/**
 * 销毁trie
 * @param trie ktrie_t实例
 * @param dtor 销毁函数
 */
extern void trie_destroy(ktrie_t* trie, knet_trie_dtor_t dtor);

/**
 * 销毁trie
 * @param trie ktrie_t实例
 * @param s 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int trie_insert(ktrie_t* trie, const char* s, void* value);

/**
 * 查找
 * @param trie ktrie_t实例
 * @param s 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int trie_find(ktrie_t* trie, const char* s, void** value);

/**
 * 删除
 * @param trie ktrie_t实例
 * @param s 键
 * @param value 值
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int trie_remove(ktrie_t* trie, const char* s, void** value);

/**
 * 遍历
 * @param trie ktrie_t实例
 * @param func 遍历函数
 * @param param 遍历函数参数
 */
extern int trie_for_each(ktrie_t* trie, knet_trie_for_each_func_t func, void* param);

/** @} */

#endif /* TRIE_API_H */
