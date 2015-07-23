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

#ifndef TRIE_API_H
#define TRIE_API_H

#include "config.h"

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

#endif /* TRIE_API_H */
