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

#ifndef FRAMEWORK_RAISER_H
#define FRAMEWORK_RAISER_H

#include "config.h"

/**
 * 建立网络连接发起者(监听器/连接器)
 * @param f kframework_t实例
 * @param loop kloop_t实例
 * @return kframework_raiser_t实例
 */
kframework_raiser_t* knet_framework_raiser_create(kframework_t* f, kloop_t* loop);

/**
 * 销毁网络连接发起者
 * @param raiser kframework_raiser_t实例
 */
void knet_framework_raiser_destroy(kframework_raiser_t* raiser);

/**
 * 启动网络连接发起者
 * @param raiser kframework_raiser_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_framework_raiser_start(kframework_raiser_t* raiser);

/**
 * 关闭网络连接发起者
 * @param raiser kframework_raiser_t实例
 */
void knet_framework_raiser_stop(kframework_raiser_t* raiser);

/**
 * 等待网络连接发起者
 * @param raiser kframework_raiser_t实例
 */
void knet_framework_raiser_wait_for_stop(kframework_raiser_t* raiser);

/**
 * 启动一个新的监听器
 * @param raiser kframework_raiser_t实例
 * @param c kframework_acceptor_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_framework_raiser_new_acceptor(kframework_raiser_t* raiser,
    kframework_acceptor_config_t* c);

/**
 * 启动一个新的连接器
 * @param raiser kframework_raiser_t实例
 * @param c kframework_connector_config_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
int knet_framework_raiser_new_connector(kframework_raiser_t* raiser,
    kframework_connector_config_t* c);

#endif /* FRAMEWORK_RAISER_H */
