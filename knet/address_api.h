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

#ifndef ADDRESS_API_H
#define ADDRESS_API_H

/**
 * @defgroup address 地址
 * 地址
 *
 * <pre>
 * 地址接口通过channel_ref_get_local_address或channel_ref_get_peer_address
 * 获取本地或对端的地址
 * </pre>
 * @sa channel_ref_get_local_address
 * @sa channel_ref_get_peer_address
 * @{
 */

/**
 * 取得IP
 * @param address address_t实例
 * @retval 有效的指针 IP字符串
 * @retval 0 管道连接未建立
 */
extern const char* address_get_ip(address_t* address);

/**
 * 取得port
 * @param address address_t实例
 * @retval 有效的端口号 端口号
 * @retval 0 管道连接未建立
 */
extern int address_get_port(address_t* address);

/** @} */

#endif /* ADDRESS_API_H */
