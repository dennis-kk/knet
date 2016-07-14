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

#ifndef LOOP_BALANCER_API_H
#define LOOP_BALANCER_API_H

#include "config.h"

/**
 * @defgroup balancer 负载均衡器
 * 负载均衡器
 *
 * <pre>
 * 负载均衡器可以与任意数量的kloop_t关联，关联后的kloop_t内监听器接受到的新管道
 * 将参与负载均衡，负载均衡的策略是kloop_t内活跃管道数量，kloop_balancer_t选择
 * 活跃管道最少的kloop_t负载新接受的管道.
 *
 * 调用knet_loop_balancer_attach让kloop_balancer_t与kloop_t关联，调用knet_loop_balancer_detach
 * 取消关联.
 * </pre>
 * @{
 */

/**
 * 创建负载均衡器
 * @return kloop_balancer_t实例
 */
extern kloop_balancer_t* knet_loop_balancer_create();

/**
 * 销毁负载均衡器
 * @param balancer kloop_balancer_t实例
 */
extern void knet_loop_balancer_destroy(kloop_balancer_t* balancer);

/**
 * 添加事件循环到负载均衡器
 * @param balancer kloop_balancer_t实例
 * @param loop kloop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_loop_balancer_attach(kloop_balancer_t* balancer, kloop_t* loop);

/**
 * 从负载均衡器内删除事件循环
 * @param balancer kloop_balancer_t实例
 * @param loop kloop_t实例
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_loop_balancer_detach(kloop_balancer_t* balancer, kloop_t* loop);

/** @} */

#endif /* LOOP_BALANCER_API_H */
