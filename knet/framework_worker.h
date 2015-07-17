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

#ifndef FRAMEWORK_WORKER_H
#define FRAMEWORK_WORKER_H

#include "config.h"

/**
 * 建立框架工作线程
 * @param f framework_t实例
 * @param loop loop_t实例
 * @return framework_worker_t实例
 */
framework_worker_t* framework_worker_create(framework_t* f, loop_t* loop);

/**
 * 销毁框架工作线程
 * @param w framework_worker_t实例
 */
void framework_worker_destroy(framework_worker_t* worker);

/**
 * 启动框架工作线程
 * @param w framework_worker_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int framework_worker_start(framework_worker_t* worker);

/**
 * 关闭框架工作线程
 * @param w framework_worker_t实例
 */
void framework_worker_stop(framework_worker_t* worker);

/**
 * 等待框架工作线程关闭
 * @param w framework_worker_t实例
 */
void framework_worker_wait_for_stop(framework_worker_t* worker);

#endif /* FRAMEWORK_WORKER_H */
