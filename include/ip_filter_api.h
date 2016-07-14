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

#ifndef IP_FILTER_API_H
#define IP_FILTER_API_H

#include "config.h"

/**
 * @defgroup ip_filter IP过滤
 * IP过滤
 * <pre>
 * 提供了快速判定指定IP是否属于IP集合接口，可以用于IP黑名单或者白名单.
 * ip_filter_t可以加载已经存在的IP过滤文件，同时也可以保存IP过滤文件，
 * IP过滤文件的格式为：
 * IP 换行
 * IP 换行
 * ......
 * 可以使用任何文本编辑器手工编辑.
 * 也可以使用接口方法实时的向过滤器内添加新的IP项或删除IP项，过滤器内容
 * 通过保存方法即可以替换旧的IP库.
 * </pre>
 * @{
 */

/**
 * 建立IP过滤器
 * @return kip_filter_t实例
 */
extern kip_filter_t* knet_ip_filter_create();

/**
 * 销毁IP过滤器
 * @param ip_filter kip_filter_t实例
 */
extern void knet_ip_filter_destroy(kip_filter_t* ip_filter);

/**
 * 加载IP过滤文件
 *
 * <pre>
 * 文件格式为:
 * [IP]\n
 * [IP]\n
 * ......
 * </pre>
 * @param ip_filter kip_filter_t实例
 * @param path 文件路径
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_ip_filter_load_file(kip_filter_t* ip_filter, const char* path);

/**
 * 添加单个IP
 * @param ip_filter kip_filter_t实例
 * @param ip IP
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_ip_filter_add(kip_filter_t* ip_filter, const char* ip);

/**
 * 删除单个IP
 * @param ip_filter kip_filter_t实例
 * @param ip IP
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_ip_filter_remove(kip_filter_t* ip_filter, const char* ip);

/**
 * 保存到文件
 * @param ip_filter kip_filter_t实例
 * @param path 文件路径
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int knet_ip_filter_save(kip_filter_t* ip_filter, const char* path);

/**
 * 检查IP是否被过滤
 * @param ip_filter kip_filter_t实例
 * @param ip IP
 * @retval 0 未被过滤
 * @retval 其他 被过滤
 */
extern int knet_ip_filter_check(kip_filter_t* ip_filter, const char* ip);

/**
 * 检查IP是否被过滤
 * @param ip_filter kip_filter_t实例
 * @param address 地址
 * @retval 0 未被过滤
 * @retval 其他 被过滤
 */
extern int knet_ip_filter_check_address(kip_filter_t* ip_filter, kaddress_t* address);

/**
 * 检查IP是否被过滤
 *
 * 过滤对端地址(peer address);
 * @param ip_filter kip_filter_t实例
 * @param channel kchannel_ref_t实例
 * @retval 0 未被过滤
 * @retval 其他 被过滤
 */
extern int knet_ip_filter_check_channel(kip_filter_t* ip_filter, kchannel_ref_t* channel);

/** @} */

#endif /* IP_FILTER_API_H */
