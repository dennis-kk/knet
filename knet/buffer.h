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

#ifndef BUFFER_H
#define BUFFER_H

#include "config.h"

/*
 * 创建一个固定长度的缓冲区
 * @param size 缓冲区长度（字节）
 * @return buffer_t实例
 */
buffer_t* buffer_create(uint32_t size);

/*
 * 销毁缓冲区
 * @param sb buffer_t实例
 */
void buffer_destroy(buffer_t* sb);

/*
 * 写入
 * @param sb buffer_t实例
 * @param temp 字节数组指针
 * @param size 字节数组长度
 * @retval 0 写入失败
 * @retval >0 实际写入的字节数
 */
uint32_t buffer_put(buffer_t* sb, const char* temp, uint32_t size);

/*
 * 取得缓冲区内数据长度
 * @param sb buffer_t实例
 * @return 数据长度
 */
uint32_t buffer_get_length(buffer_t* sb);

/*
 * 取得缓冲区数据起始地址
 * @param sb buffer_t实例
 * @return 数据长度
 */
char* buffer_get_ptr(buffer_t* sb);

/*
 * 调整数据起始地址
 * @param sb buffer_t实例
 * @param gap 调整的长度
 */
void buffer_adjust(buffer_t* sb, uint32_t gap);

#endif /* BUFFER_H */
