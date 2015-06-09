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

#ifndef STREAM_API_H
#define STREAM_API_H

#include "config.h"

/*
 * 取得数据流内可读字节数
 * @param stream stream_t实例
 * @return 可读字节数
 */
extern int stream_available(stream_t* stream);

/*
 * 清空数据流
 * @param stream stream_t实例
 */
extern void stream_eat(stream_t* stream);

/*
 * 从数据流内读取数据并清除数据
 * @param stream stream_t实例
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 读到的字节数
 */
extern int stream_pop(stream_t* stream, char* buffer, int size);

/*
 * 向数据流内写数据
 * @param stream stream_t实例
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 写入的字节数
 */
extern int stream_push(stream_t* stream, char* buffer, int size);

/*
 * 从数据流内拷贝数据，但不清除数据流内数据
 * @param stream stream_t实例
 * @param buffer 缓冲区
 * @param size 缓冲区大小
 * @return 拷贝的字节数
 */
extern int stream_copy(stream_t* stream, char* buffer, int size);

#endif /* STREAM_API_H */
