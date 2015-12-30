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

#ifndef MISC_API_H
#define MISC_API_H

#include "config.h"

/**
 * 获取当前毫秒
 */
extern uint32_t time_get_milliseconds();

/**
 * 获取当前微秒
 */
extern uint64_t time_get_microseconds();

/**
 * gettimeofday
 * @sa gettimeofday
 */
extern int time_gettimeofday(struct timeval *tp, void *tzp);

/**
 * 取得当前可阅读时间字符串
 * @param buffer 输出缓冲区
 * @param size 缓冲区大小
 * @return 格式为YYYY-MM-DD HH:mm:SS:MS
 */
extern char* time_get_string(char* buffer, int size);

/**
 * 产生一个伪UUID，只保证本进程内不重复
 * @return 伪UUID
 */
extern uint64_t uuid_create();

/**
 * 取得UUID高32位
 * @param uuid UUID
 * @return 高32位
 */
extern uint32_t uuid_get_high32(uint64_t uuid);

/**
 * 取得当前工作目录
 * @param buffer 路径缓冲区指针
 * @param size 缓冲区大小
 * @retval 0 失败
 * @retval 路径缓冲区指针
 */
extern char* path_getcwd(char* buffer, int size);

/**
 * 获取最新的系统错误码
 * @return 系统错误码
 */
extern sys_error_t sys_get_errno();

/**
 * 字节序转换 - 主机序到网络序
 * @param ui64 64位无符号整数
 * @return 64位无符号整数
 */
extern uint64_t knet_htonll(uint64_t ui64);

#ifndef htonll
#define htonll knet_htonll
#endif /* htonll */

/**
 * 字节序转换 - 网络序到主机序
 * @param ui64 64位无符号整数
 * @return 64位无符号整数
 */
extern uint64_t knet_ntohll(uint64_t ui64);

#ifndef ntohll
#define ntohll knet_ntohll
#endif /* ntohll */

/**
 * 取得管道回调事件描述
 * @param e 管道回调事件ID
 * @return 管道回调事件描述
 */
extern const char* get_channel_cb_event_string(knet_channel_cb_event_e e);

/**
 * 取得管道回调事件名字
 * @param e 管道回调事件ID
 * @return 管道回调事件名字
 */
extern const char* get_channel_cb_event_name(knet_channel_cb_event_e e);

/**
 * long转为char*
 * @param l long
 * @param buffer 存储转换的字符串
 * @param size 缓冲区长度
 * @retval 0 失败
 * @retval 其他 成功
 */
extern char* knet_ltoa(long l, char* buffer, int size);

/**
 * long long 转为char*
 * @param ll long long
 * @param buffer 存储转换的字符串
 * @param size 缓冲区长度
 * @retval 0 失败
 * @retval 其他 成功
 */
extern char* knet_lltoa(long long ll, char* buffer, int size);

/**
 * 分割字符串
 * @param src 待分割字符串
 * @param delim 分割字符
 * @param n 分割后子串数量
 * @retval 0 成功
 * @retval 其他 失败
 */
extern int split(const char* src, char delim, int n, ...);

/**
 * 获取主机域名的IP
 * @param host_name 主机域名
 * @param ip 返回IP字符串
 * @param size 返回缓冲区长度
 * @retval error_ok 成功
 * @retval 其他 失败
 */
extern int get_host_ip_string(const char* host_name, char* ip, int size);

/**
* 字符串转long long
* @param p 字符串
* @return long long
*/
extern long long knet_atoll(const char *p);

#if defined(WIN32)
#define atoll knet_atoll
#endif /* defined(WIN32) && !defined(atoll) */

#endif /* MISC_API_H */
