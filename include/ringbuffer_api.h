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

#ifndef RINGBUFFER_API_H
#define RINGBUFFER_API_H

#include "config.h"


/**
 * 建立一个ringbuffer
 * @param size 最大长度
 * @return kringbuffer_t实例
 */
extern kringbuffer_t* ringbuffer_create(uint32_t size);

/**
 * 销毁ringbuffer
 * @param rb kringbuffer_t实例
 */
extern void ringbuffer_destroy(kringbuffer_t* rb);

/**
 * 读取并清除
 * @param rb kringbuffer_t实例
 * @param buffer 写入缓冲区指针
 * @param size 写入缓冲区长度
 * @return 实际读出字节数
 */
extern uint32_t ringbuffer_read(kringbuffer_t* rb, char* buffer, uint32_t size);

/**
 * 写入
 * @param rb kringbuffer_t实例
 * @param buffer 写入缓冲区指针
 * @param size 写入缓冲区长度
 * @return 实际写入字节数
 */
extern uint32_t ringbuffer_write(kringbuffer_t* rb, const char* buffer, uint32_t size);

/**
 * 替换
 * @param rb kringbuffer_t实例
 * @param pos 替换的起始位置
 * @param buffer 写入缓冲区指针
 * @param size 写入缓冲区长度
 * @return 实际写入字节数
 */
extern uint32_t ringbuffer_replace(kringbuffer_t* rb, uint32_t pos, const char* buffer, uint32_t size);

/**
 * 读取但不清除
 * @param rb kringbuffer_t实例
 * @param buffer 写入缓冲区指针
 * @param size 写入缓冲区长度
 * @return 实际读出字节数
 */
extern uint32_t ringbuffer_copy(kringbuffer_t* rb, char* buffer, uint32_t size);

/**
 * 查找指定的目标，并返回位置
 * @param rb kringbuffer_t实例
 * @param target 目标字符串
 * @param size 位置
 * @retval error_ok 找到
 * @retval 其他 未找到
 */
extern uint32_t ringbuffer_find(kringbuffer_t* rb, const char* target, uint32_t* size);

/**
 * 取得可读字节数
 * @param rb kringbuffer_t实例
 * @return 可读字节数
 */
extern uint32_t ringbuffer_available(kringbuffer_t* rb);

/**
 * 清除所有可读字节
 * @param rb kringbuffer_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int ringbuffer_eat_all(kringbuffer_t* rb);

/**
 * 清除指定长度的可读字节
 * @param rb kringbuffer_t实例
 * @param size 需要清除的长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int ringbuffer_eat(kringbuffer_t* rb, uint32_t size);

/**
 * 取得非绕回连续地址的最大可读字节数
 * @param rb kringbuffer_t实例
 * @return 非绕回连续地址的最大可读字节数
 */
extern uint32_t ringbuffer_read_lock_size(kringbuffer_t* rb);

/**
 * 取得可读数据起止指针
 * @param rb kringbuffer_t实例
 * @return 可读数据起止指针
 */
extern char* ringbuffer_read_lock_ptr(kringbuffer_t* rb);

/**
 * 提交并清除已经读到的字节
 * @param rb kringbuffer_t实例
 * @param size 已经读出的字节数
 */
extern void ringbuffer_read_commit(kringbuffer_t* rb, uint32_t size);

/**
 * 虚拟窗口 - 取得非绕回连续地址的最大可读字节数
 * @param rb kringbuffer_t实例
 * @return 非绕回连续地址的最大可读字节数
 */
extern uint32_t ringbuffer_window_read_lock_size(kringbuffer_t* rb);

/**
 * 虚拟窗口 - 取得可读数据起止指针
 * @param rb kringbuffer_t实例
 * @return 可读数据起止指针
 */
extern char* ringbuffer_window_read_lock_ptr(kringbuffer_t* rb);

/**
 * 虚拟窗口 - 提交已经读到的字节，但不清除
 * @param rb kringbuffer_t实例
 * @param size 已经读出的字节数
 */
void ringbuffer_window_read_commit(kringbuffer_t* rb, uint32_t size);

/**
 * 取得非绕回可连续写入的最大长度
 * @param rb kringbuffer_t实例
 * @return 非绕回可连续写入的最大长度
 */
extern uint32_t ringbuffer_write_lock_size(kringbuffer_t* rb);

/**
 * 取得可写起始指针
 * @param rb kringbuffer_t实例
 * @return 可写起止指针
 */
extern char* ringbuffer_write_lock_ptr(kringbuffer_t* rb);

/**
 * 提交成功写入的字节数
 * @param rb kringbuffer_t实例
 * @param size 成功写入的字节数
 */
extern void ringbuffer_write_commit(kringbuffer_t* rb, uint32_t size);

/**
 * 满
 * @param rb kringbuffer_t实例
 * @retval 0 未满
 * @retval 非零 满
 */
extern int ringbuffer_full(kringbuffer_t* rb);

/**
 * 空
 * @param rb kringbuffer_t实例
 * @retval 0 非空
 * @retval 非零 空
 */
extern int ringbuffer_empty(kringbuffer_t* rb);

/**
 * 取得最大长度
 * @param rb kringbuffer_t实例
 * @return 最大长度
 */
extern uint32_t ringbuffer_get_max_size(kringbuffer_t* rb);

/**
 * 将内容打印到屏幕
 * @param rb kringbuffer_t实例
 */
extern void ringbuffer_print_stdout(kringbuffer_t* rb);

#endif /* RINGBUFFER_API_H */
