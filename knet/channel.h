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

#ifndef CHANNEL_H
#define CHANNEL_H

#include "config.h"

/**
 * ����һ��kchannel_tʵ��
 * @param max_send_list_len ����������󳤶�
 * @param recv_ring_len ���ܻ�������󳤶�
 * @param ipv6 �Ƿ���IPV6
 * @return kchannel_tʵ��
 */
kchannel_t* knet_channel_create(uint32_t max_send_list_len, uint32_t recv_ring_len, int ipv6);

/**
 * ����һ��kchannel_tʵ��
 * @param socket_fd �ѽ������׽���
 * @param max_send_list_len ����������󳤶�
 * @param recv_ring_len ���ܻ�������󳤶�
 * @param ipv6 �Ƿ���IPV6
 * @return kchannel_tʵ��
 */
kchannel_t* knet_channel_create_exist_socket_fd(socket_t socket_fd, uint32_t max_send_list_len, uint32_t recv_ring_len, int ipv6);

/**
 * ����kchannel_tʵ��
 * @param channel kchannel_tʵ��
 */
void knet_channel_destroy(kchannel_t* channel);

/**
 * ���Ӽ�����
 * @param channel kchannel_tʵ��
 * @param ip IP
 * @param port �˿�
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_connect(kchannel_t* channel, const char* ip, int port);

/**
 * ����
 * @param channel kchannel_tʵ��
 * @param ip IP
 * @param port �˿�
 * @param backlog �ȴ����г���
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_accept(kchannel_t* channel, const char* ip, int port, int backlog);

/**
 * �ر�
 * @param channel kchannel_tʵ��
 */
void knet_channel_close(kchannel_t* channel);

/**
 * ����
 * ����������Ϊ�յ�ʱ�򣬻����ȳ���ֱ�ӷ��͵��׽��ֻ�����(zero copy)�������ŵ���������ĩβ�ȴ�
 * �ʵ�ʱ������.
 * @param channel kchannel_tʵ��
 * @param data ��������ָ��
 * @param size ���ݳ���
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_send(kchannel_t* channel, const char* data, int size);

/**
 * ����
 * �ŵ���������ĩβ�ȴ��ʵ�ʱ������.
 * @param channel kchannel_tʵ��
 * @param send_buffer ���ͻ�����kbuffer_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_send_buffer(kchannel_t* channel, kbuffer_t* send_buffer);

/**
 * ��д�¼�֪ͨ
 * @param channel kchannel_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_update_send(kchannel_t* channel);

/**
 * �ɶ��¼�֪ͨ
 * @param channel kchannel_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_update_recv(kchannel_t* channel);

/**
 * ȡ���׽���
 * @param channel kchannel_tʵ��
 * @return �׽���
 */
socket_t knet_channel_get_socket_fd(kchannel_t* channel);

/**
 * ȡ�ö�������
 * @param channel kchannel_tʵ��
 * @return kringbuffer_tʵ��
 */
kringbuffer_t* knet_channel_get_ringbuffer(kchannel_t* channel);

/**
 * ȡ�÷���������󳤶�����
 * @param channel kchannel_tʵ��
 * @return ����������󳤶�����
 */
uint32_t knet_channel_get_max_send_list_len(kchannel_t* channel);

/**
 * ȡ�ý��ջ�������󳤶�����
 * @param channel kchannel_tʵ��
 * @return ���ջ�������󳤶�����
 */
uint32_t knet_channel_get_max_recv_buffer_len(kchannel_t* channel);

/**
 * ��ȡ�ܵ�UUID
 * @param channel kchannel_tʵ��
 * @return �ܵ�UUID
 */
uint64_t knet_channel_get_uuid(kchannel_t* channel);

/**
 * ���������ڻ����������Ƿ�ﵽ���
 * @param channel kchannel_tʵ��
 * @retval 0 ����
 * @retval ���� ��
 */
int knet_channel_send_list_reach_max(kchannel_t* channel);

/**
 * �Ƿ���IPV6
 * @param channel kchannel_tʵ��
 * @retval 0 ����
 * @retval ���� ��
 */
int knet_channel_is_ipv6(kchannel_t* channel);

#endif /* CHANNEL_H */