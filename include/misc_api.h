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

#ifndef MISC_API_H
#define MISC_API_H

#include "config.h"

/**
 * ��ȡ��ǰ����
 */
extern uint64_t time_get_milliseconds();

/**
 * ��ȡ��ǰ΢��
 */
extern uint64_t time_get_microseconds();

/**
 * ��ȡ��1970��1��1�յ����ڵĺ�����
 */
extern uint64_t time_get_milliseconds_19700101();

/**
 * localtime
 * @see localtime_s or localtime_r
 */
extern void knet_localtime(struct tm* tm, const time_t* time);

/**
 * gettimeofday
 * @sa gettimeofday
 */
extern int time_gettimeofday(struct timeval *tp, void *tzp);

/**
 * ȡ�õ�ǰ���Ķ�ʱ���ַ���
 * @param buffer ���������
 * @param size ��������С
 * @return ��ʽΪYYYY-MM-DD HH:mm:SS:MS
 */
extern char* time_get_string(char* buffer, int size);

/**
 * ����һ��αUUID��ֻ��֤�������ڲ��ظ�
 * @return αUUID
 */
extern uint64_t uuid_create();

/**
 * ȡ��UUID��32λ
 * @param uuid UUID
 * @return ��32λ
 */
extern uint32_t uuid_get_high32(uint64_t uuid);

/**
 * ȡ�õ�ǰ����Ŀ¼
 * @param buffer ·��������ָ��
 * @param size ��������С
 * @retval 0 ʧ��
 * @retval ·��������ָ��
 */
extern char* path_getcwd(char* buffer, int size);

/**
 * ��ȡ���µ�ϵͳ������
 * @return ϵͳ������
 */
extern sys_error_t sys_get_errno();

/**
 * �ֽ���ת�� - ������������
 * @param ui64 64λ�޷�������
 * @return 64λ�޷�������
 */
extern uint64_t knet_htonll(uint64_t ui64);

#ifndef htonll
#define htonll knet_htonll
#endif /* htonll */

/**
 * �ֽ���ת�� - ������������
 * @param ui64 64λ�޷�������
 * @return 64λ�޷�������
 */
extern uint64_t knet_ntohll(uint64_t ui64);

#ifndef ntohll
#define ntohll knet_ntohll
#endif /* ntohll */

/**
 * ȡ�ùܵ��ص��¼�����
 * @param e �ܵ��ص��¼�ID
 * @return �ܵ��ص��¼�����
 */
extern const char* get_channel_cb_event_string(knet_channel_cb_event_e e);

/**
 * ȡ�ùܵ��ص��¼�����
 * @param e �ܵ��ص��¼�ID
 * @return �ܵ��ص��¼�����
 */
extern const char* get_channel_cb_event_name(knet_channel_cb_event_e e);

/**
 * longתΪchar*
 * @param l long
 * @param buffer �洢ת�����ַ���
 * @param size ����������
 * @retval 0 ʧ��
 * @retval ���� �ɹ�
 */
extern char* knet_ltoa(long l, char* buffer, int size);

/**
 * long long תΪchar*
 * @param ll long long
 * @param buffer �洢ת�����ַ���
 * @param size ����������
 * @retval 0 ʧ��
 * @retval ���� �ɹ�
 */
extern char* knet_lltoa(long long ll, char* buffer, int size);

/**
 * �ָ��ַ���
 * @param src ���ָ��ַ���
 * @param delim �ָ��ַ�
 * @param n �ָ���Ӵ�����
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
extern int split(const char* src, char delim, int n, ...);

/**
 * ��ȡ����������IP
 * @param host_name ��������
 * @param ip ����IP�ַ���
 * @param size ���ػ���������
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int get_host_ip_string(const char* host_name, char* ip, int size);

/**
 * �ַ���תlong long
 * @param p �ַ���
 * @return long long
 */
extern long long knet_atoll(const char *p);

#if (defined(WIN32) || defined(_WIN64))
#define atoll knet_atoll
#endif /* defined(WIN32) && !defined(atoll) */

/**
 * ����malloc����ָ��
 * @param func ����ָ��
 */
extern void knet_set_malloc_func(knet_malloc_func_t func);

/**
 * ����realloc����ָ��
 * @param func ����ָ��
 */
extern void knet_set_realloc_func(knet_realloc_func_t func);

/**
 * ����free����ָ��
 * @param func ����ָ��
 */
extern void knet_set_free_func(knet_free_func_t func);

#endif /* MISC_API_H */
