
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

#ifndef MISC_H
#define MISC_H

#include "config.h"
#include "thread_api.h"
#include "misc_api.h"

/**
 * ����һ���׽���
 * @return �׽���
 */
socket_t socket_create();

/**
 * ����һ���׽���, IPV6
 * @return �׽���
 */
socket_t socket_create6();

/**
 * �����첽connect
 * @param socket_fd �׽���
 * @param ip IP
 * @param port �˿�
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int socket_connect(socket_t socket_fd, const char* ip, int port);

/**
 * �����첽connect, IPV6
 * @param socket_fd �׽���
 * @param ip IP
 * @param port �˿�
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int socket_connect6(socket_t socket_fd, const char* ip, int port);

/**
 * bind & listen
 * @param socket_fd �׽���
 * @param ip IP
 * @param port �˿�
 * @param backlog ����listen()����
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int socket_bind_and_listen(socket_t socket_fd, const char* ip, int port, int backlog);

/**
 * bind & listen, IPV6
 * @param socket_fd �׽���
 * @param ip IP
 * @param port �˿�
 * @param backlog ����listen()����
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int socket_bind_and_listen6(socket_t socket_fd, const char* ip, int port, int backlog);

/**
 * accept
 * @param socket_fd �׽���
 * @retval 0 ʧ��
 * @retval ��Ч���׽���
 */
socket_t socket_accept(socket_t socket_fd);

/**
 * accept, IPV6
 * @param socket_fd �׽���
 * @retval 0 ʧ��
 * @retval ��Ч���׽���
 */
socket_t socket_accept6(socket_t socket_fd);

/**
 * �ر��׽��֣�ǿ�ƹرգ�
 * @param socket_fd �׽���
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_close(socket_t socket_fd);

/**
 * ������ַ����
 * @param socket_fd
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_reuse_addr_on(socket_t socket_fd);

/**
 * �����׽��ַ�����
 * @param socket_fd
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_non_blocking_on(socket_t socket_fd);

/**
 * �ر�nagle�㷨
 *
 * �����ݸ���ı�����
 * @param socket_fd
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_nagle_off(socket_t socket_fd);

/**
 * �ر�linger
 *
 * �����ѹرյĶ˿ڽ���TIME_WAIT״̬
 * @param socket_fd
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_linger_off(socket_t socket_fd);

/**
 * �ر�keep-alive
 * @param socket_fd
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_keepalive_off(socket_t socket_fd);

/**
 * ����SO_DONTROUTE��δʹ��
 * @param socket_fd
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_donot_route_on(socket_t socket_fd);

/**
 * ���ý��ջ�������С
 * @param socket_fd
 * @param size ���ջ�������С
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_recv_buffer_size(socket_t socket_fd, int size);

/**
 * ���÷��ͻ�������С
 * @param socket_fd
 * @param size ���ͻ�������С
 * @retval 0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_set_send_buffer_size(socket_t socket_fd, int size);

/**
 * ����
 * @param socket_fd
 * @param data ����ָ��
 * @param size ���ݳ���
 * @retval >0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_send(socket_t socket_fd, const char* data, uint32_t size);

/**
 * ����
 * @param socket_fd
 * @param data ���ջ�����ָ��
 * @param size ���ջ��峤��
 * @retval >0 �ɹ�
 * @retval ���� ʧ��
 */
int socket_recv(socket_t socket_fd, char* data, uint32_t size);

/**
 * socketpair
 * @sa socketpair
 */
int socket_pair(socket_t pair[2]);

/**
 * getpeername
 * @sa getpeername
 */
int socket_getpeername(kchannel_ref_t* channel_ref, kaddress_t* address);

/**
 * getpeername, IPV6
 * @sa getpeername
 */
int socket_getpeername6(kchannel_ref_t* channel_ref, kaddress_t* address);

/**
 * getsockname
 * @sa getsockname
 */
int socket_getsockname(kchannel_ref_t* channel_ref, kaddress_t* address);

/**
 * getsockname, IPV6
 * @sa getsockname
 */
int socket_getsockname6(kchannel_ref_t* channel_ref, kaddress_t* address);

/**
 * ����׽����Ƿ��д
 * @retval 0 ����д
 * @retval 1 ��д
 */
int socket_check_send_ready(socket_t socket_fd);

#endif /* MISC_H */
