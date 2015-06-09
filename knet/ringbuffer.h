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

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "config.h"

/*
 * 建立一个ringbuffer
 * @param size 最大长度
 * @return ringbuffer_t实例
 */
ringbuffer_t* ringbuffer_create(uint32_t size);

/*
 * 销毁ringbuffer
 * @param rb ringbuffer_t实例
 */
void ringbuffer_destroy(ringbuffer_t* rb);

/*
 * 读取并清除
 * @param rb ringbuffer_t实例
 * @param buffer 写入缓冲区指针
 * @param size 写入缓冲区长度
 * @return 实际读出字节数
 */
uint32_t ringbuffer_read(ringbuffer_t* rb, char* buffer, uint32_t size);

/*
 * 读取但不清除
 * @param rb ringbuffer_t实例
 * @param buffer 写入缓冲区指针
 * @param size 写入缓冲区长度
 * @return 实际读出字节数
 */
uint32_t ringbuffer_copy(ringbuffer_t* rb, char* buffer, uint32_t size);

/*
 * 取得可读字节数
 * @param rb ringbuffer_t实例
 * @return 可读字节数
 */
uint32_t ringbuffer_available(ringbuffer_t* rb);

/*
 * 清除所有可读字节
 * @param rb ringbuffer_t实例
 */
void ringbuffer_eat(ringbuffer_t* rb);

/*
 * 取得非绕回连续地址的最大可读字节数
 * @param rb ringbuffer_t实例
 * @return 非绕回连续地址的最大可读字节数
 */
uint32_t ringbuffer_read_lock_size(ringbuffer_t* rb);

/*
 * 取得可读数据起止指针
 * @param rb ringbuffer_t实例
 * @return 可读数据起止指针
 */
char* ringbuffer_read_lock_ptr(ringbuffer_t* rb);

/*
 * 提交并清除已经读到的字节
 * @param rb ringbuffer_t实例
 * @param size 已经读出的字节数
 */
void ringbuffer_read_commit(ringbuffer_t* rb, uint32_t size);

/*
 * 取得非绕回可连续写入的最大长度
 * @param rb ringbuffer_t实例
 * @return 非绕回可连续写入的最大长度
 */
uint32_t ringbuffer_write_lock_size(ringbuffer_t* rb);

/*
 * 取得可写起止指针
 * @param rb ringbuffer_t实例
 * @return 可写起止指针
 */
char* ringbuffer_write_lock_ptr(ringbuffer_t* rb);

/*
 * 提交成功写入的字节数
 * @param rb ringbuffer_t实例
 * @param size 成功写入的字节数
 */
void ringbuffer_write_commit(ringbuffer_t* rb, uint32_t size);

/*
 * 满
 * @param rb ringbuffer_t实例
 * @retval 0 未满
 * @retval 非零 满
 */
int ringbuffer_full(ringbuffer_t* rb);

/*
 * 空
 * @param rb ringbuffer_t实例
 * @retval 0 非空
 * @retval 非零 空
 */
int ringbuffer_empty(ringbuffer_t* rb);

/*
 * 取得最大长度
 * @param rb ringbuffer_t实例
 * @return 最大长度
 */
uint32_t ringbuffer_get_max_size(ringbuffer_t* rb);

#endif /* RINGBUFFER_H */
