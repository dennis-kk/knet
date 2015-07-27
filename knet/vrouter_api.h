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

#ifndef VROUTER_API_H
#define VROUTER_API_H

#include "config.h"

/**
 * @defgroup vrouter 虚拟路由
 * 虚拟路由
 *
 * <pre>
 * 提供一个点对点单向的路由关系表，表内维护了管道路由的{c1, c2}的对应关系，
 * 但这个转发关系是单向的，即只支持c1到c2的转发，但不至此后c2到c1的转发，
 * 如果要支持c2到c1的转发，需要添加新的转发关系{c2, c1}.
 * 表内使用源管道的UUID作为键，所以同一个管道作为起始管道只能出现一次,但
 * 作为目的管道可以出现N次.
 * 所有建立转发关系的管道对都会被提升引用计数，从而控制管道的声明周期，防止在
 * 外部销毁内部正在使用的管道.
 * </pre>
 * @{
 */

/**
 * 建立虚拟连接路由器
 * @return kvrouter_t实例
 */
extern kvrouter_t* knet_vrouter_create();

/**
 * 销毁
 * return kvrouter_t实例
 */
extern void knet_vrouter_destroy(kvrouter_t* router);

/**
 * 建立一条转发转发关系
 * @param router kvrouter_t实例
 * @param c1 kchannel_ref_t实例，源管道
 * @param c2 kchannel_ref_t实例，目的管道
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_vrouter_add_wire(kvrouter_t* router, kchannel_ref_t* c1, kchannel_ref_t* c2);

/**
 * 删除一条转发关系
 * @param router kvrouter_t实例
 * @param c kchannel_ref_t实例，源管道(knet_vrouter_add_wire第二个参数)
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_vrouter_remove_wire(kvrouter_t* router, kchannel_ref_t* c);

/**
 * 转发数据
 * @param router kvrouter_t实例
 * @param c kchannel_ref_t实例，源管道(knet_vrouter_add_wire第二个参数)
 * @param buffer 数据缓冲区
 * @param size 缓冲区长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_vrouter_route(kvrouter_t* router, kchannel_ref_t* c, void* buffer, int size);

/** @} */

#endif /* VROUTER_API_H */
