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

#ifndef CHANNEL_REF_H
#define CHANNEL_REF_H

#include "config.h"
#include "channel_ref_api.h"

/**
 * �����ܵ�����
 * @param loop kloop_tʵ��
 * @param channel kchannel_tʵ��
 * @return kchannel_ref_tʵ��
 */
kchannel_ref_t* knet_channel_ref_create(kloop_t* loop, kchannel_t* channel);

/**
 * ���ٹܵ�����
 * �ܵ����ü���Ϊ��ʱ���ܱ�ʵ������
 * @param channel_ref kchannel_ref_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_ref_destroy(kchannel_ref_t* channel_ref);

/**
 * �رչܵ����������
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_close_check_reconnect(kchannel_ref_t* channel_ref);

/**
 * д��
 * �ܵ����ü���Ϊ��ʱ���ܱ�ʵ������
 * @param channel_ref kchannel_ref_tʵ��
 * @param data д������ָ��
 * @param size ���ݳ���
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_ref_write(kchannel_ref_t* channel_ref, const char* data, int size);

/**
 * Ϊͨ��accept()���ص��׽��ִ����ܵ�����
 * @param channel_ref kchannel_ref_tʵ��
 * @param loop kloop_tʵ��
 * @param client_fd ͨ��accept()�õ����׽���
 * @param event �Ƿ�Ͷ���¼������ùܵ�״̬
 * @param ipv6 �Ƿ���IPV6
 * @return kchannel_ref_tʵ��
 */
kchannel_ref_t* knet_channel_ref_accept_from_socket_fd(kchannel_ref_t* channel_ref, kloop_t* loop, socket_t client_fd, int event, int ipv6);

/**
 * ȡ�ùܵ��������kloop_tʵ��
 * @param channel_ref kchannel_ref_tʵ��
 * @return kloop_tʵ��
 */
kloop_t* knet_channel_ref_choose_loop(kchannel_ref_t* channel_ref);

/**
 * ���ùܵ�����ڵ�
 * @param channel_ref kchannel_ref_tʵ��
 * @param node ����ڵ�
 */
void knet_channel_ref_set_loop_node(kchannel_ref_t* channel_ref, kdlist_node_t* node);

/**
 * ȡ�ùܵ�����ڵ�
 * @param channel_ref kchannel_ref_tʵ��
 * @return kdlist_node_tʵ��
 */
kdlist_node_t* knet_channel_ref_get_loop_node(kchannel_ref_t* channel_ref);

/**
 * ��kloop_t�����е��߳��������������
 * @param channel_ref kchannel_ref_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_ref_connect_in_loop(kchannel_ref_t* channel_ref);

/**
 * ��kloop_t�����е��߳��������������
 * @param channel_ref kchannel_ref_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_ref_connect_in_loop_address(kchannel_ref_t* channel_ref);

/**
 * ��kloop_t�����е��߳�����ɽ�������������
 * ͨ�����ؾ��ⴥ��
 * @param loop kloop_tʵ��
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_update_accept_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * �����������listen()��bind()�������ڵ�ǰ�̵߳�kloop_t�ڼ���
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_accept_async(kchannel_ref_t* channel_ref);

/**
 * ��kloop_t�����е��߳�����ɹر�����
 * ͨ�����̹߳رմ���
 * @param loop kloop_tʵ��
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_update_close_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref);

/**
 * ��kloop_t�����е��߳��ڷ���
 * ͨ�����̷߳��ʹ���
 * @param loop kloop_tʵ��
 * @param channel_ref kchannel_ref_tʵ��
 * @param send_buffer kbuffer_tʵ��
 */
void knet_channel_ref_update_send_in_loop(kloop_t* loop, kchannel_ref_t* channel_ref, kbuffer_t* send_buffer);

/**
 * ���ùܵ��Զ����־
 * @param channel_ref kchannel_ref_tʵ��
 * @param flag �Զ����־
 */
void knet_channel_ref_set_flag(kchannel_ref_t* channel_ref, int flag);

/**
 * ȡ�ùܵ��Զ����־
 * @param channel_ref kchannel_ref_tʵ��
 * @return �Զ����־
 */
int knet_channel_ref_get_flag(kchannel_ref_t* channel_ref);

/**
 * ���ùܵ��Զ�������
 * @param channel_ref kchannel_ref_tʵ��
 * @param data �Զ�������ָ��
 */
void knet_channel_ref_set_data(kchannel_ref_t* channel_ref, void* data);

/**
 * ȡ�ùܵ��Զ�������
 * @param channel_ref kchannel_ref_tʵ��
 * @return �Զ�������ָ��
 */
void* knet_channel_ref_get_data(kchannel_ref_t* channel_ref);

/**
 * ���ùܵ��������������kloop_tʵ��
 * @param channel_ref kchannel_ref_tʵ��
 * @param loop kloop_tʵ��
 */
void knet_channel_ref_set_loop(kchannel_ref_t* channel_ref, kloop_t* loop);

/**
 * Ͷ�ݹܵ��¼�
 * @param channel_ref kchannel_ref_tʵ��
 * @param e �ܵ��¼�
 */
void knet_channel_ref_set_event(kchannel_ref_t* channel_ref, knet_channel_event_e e);

/**
 * ��ȡ�ܵ��Ѿ�Ͷ�ݵ��¼�����
 * @param channel_ref kchannel_ref_tʵ��
 * @return �ܵ��¼�����
 */
knet_channel_event_e knet_channel_ref_get_event(kchannel_ref_t* channel_ref);

/**
 * ȡ���ܵ��¼�
 * @param channel_ref kchannel_ref_tʵ��
 * @param e �ܵ��¼�
 */
void knet_channel_ref_clear_event(kchannel_ref_t* channel_ref, knet_channel_event_e e);

/**
 * ����Ƿ�Ͷ�����¼�
 * @param channel_ref kchannel_ref_tʵ��
 * @param event �ܵ��¼�
 * @retval 0 û��Ͷ��
 * @retval ���� �Ѿ�Ͷ��
 */
int knet_channel_ref_check_event(kchannel_ref_t* channel_ref, knet_channel_event_e event);

/**
 * ���ùܵ�״̬
 * @param channel_ref kchannel_ref_tʵ��
 * @param state �ܵ�״̬
 */
void knet_channel_ref_set_state(kchannel_ref_t* channel_ref, knet_channel_state_e state);

/**
 * �ܵ��¼�֪ͨ
 * @param channel_ref kchannel_ref_tʵ��
 * @param e �ܵ��¼�
 * @param ts ��ǰʱ������룩
 */
void knet_channel_ref_update(kchannel_ref_t* channel_ref, knet_channel_event_e e, time_t ts);

/**
 * �ܵ��¼�����-����������������
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_update_accept(kchannel_ref_t* channel_ref);

/**
 * �ܵ��¼�����-�����������
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_update_connect(kchannel_ref_t* channel_ref);

/**
 * �ܵ��¼�����-�����ݿɶ�
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_update_recv(kchannel_ref_t* channel_ref);

/**
 * �ܵ��¼�����-���Է�������
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_update_send(kchannel_ref_t* channel_ref);

/**
 * ��ȡ�ܵ������г�ʱ
 * @param channel_ref kchannel_ref_tʵ��
 * @return �ܵ������г�ʱ
 */
int knet_channel_ref_get_timeout(kchannel_ref_t* channel_ref);

/**
 * ��ȡ�ܵ����ӳ�ʱ
 * @param channel_ref kchannel_ref_tʵ��
 * @return �ܵ����ӳ�ʱ
 */
int knet_channel_ref_get_connect_timeout(kchannel_ref_t* channel_ref);

/**
 * ȡ�ùܵ���������
 * @param channel_ref kchannel_ref_tʵ��
 * @return kringbuffer_tʵ��
 */
kringbuffer_t* knet_channel_ref_get_ringbuffer(kchannel_ref_t* channel_ref);

/**
 * ȡ�ùܵ��¼��ص�
 * @param channel_ref kchannel_ref_tʵ��
 * @return �ص�����ָ��
 */
knet_channel_ref_cb_t knet_channel_ref_get_cb(kchannel_ref_t* channel_ref);

/**
 * ����������ڵ�
 * @param channel_ref kchannel_ref_tʵ��
 * @param node ������ڵ�
 */
void knet_channel_ref_set_domain_node(kchannel_ref_t* channel_ref, kdlist_node_t* node);

/**
 * ȡ��������ڵ�
 * @param channel_ref kchannel_ref_tʵ��
 * @return kdlist_node_tʵ��
 */
kdlist_node_t* knet_channel_ref_get_domain_node(kchannel_ref_t* channel_ref);

/**
 * ���ܵ������Ƿ�ͨ������knet_channel_ref_share()����
 * @param channel_ref kchannel_ref_tʵ��
 * @retval 0 ����
 * @retval ���� ��
 */
int knet_channel_ref_check_share(kchannel_ref_t* channel_ref);

/**
 * ������ID
 * @param channel_ref kchannel_ref_tʵ��
 * @param domain_id ��ID
 */
void knet_channel_ref_set_domain_id(kchannel_ref_t* channel_ref, uint64_t domain_id);

/**
 * ȡ����ID
 * @param channel_ref kchannel_ref_tʵ��
 * @return ��ID
 */
uint64_t knet_channel_ref_get_domain_id(kchannel_ref_t* channel_ref);

/**
 * ���Թܵ������Ƿ�Ϊ��
 * @param channel_ref kchannel_ref_tʵ��
 * @return 0 ��Ϊ��
 * @return ���� Ϊ��
 */
int knet_channel_ref_check_ref_zero(kchannel_ref_t* channel_ref);

/**
 * ��ȡ�ܵ����ü���
 * @param channel_ref kchannel_ref_tʵ��
 * @return �ܵ����ü���
 */
int knet_channel_ref_get_ref(kchannel_ref_t* channel_ref);

/**
 * ���ùܵ��û�����
 * @param channel_ref kchannel_ref_tʵ��
 * @param data �û�����ָ��
 */
void knet_channel_ref_set_user_data(kchannel_ref_t* channel_ref, void* data);

/**
 * ��ȡ�ܵ��û�����
 * @param channel_ref kchannel_ref_tʵ��
 * @return �û�����ָ��
 */
void* knet_channel_ref_get_user_data(kchannel_ref_t* channel_ref);

/**
 * ���ý��ճ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 * @param timer ��ʱ��
 */
void knet_channel_ref_set_recv_timeout_timer(kchannel_ref_t* channel_ref, ktimer_t* timer);

/**
 * ��ȡ���ճ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 * @return ��ʱ��
 */
ktimer_t* knet_channel_ref_get_recv_timeout_timer(kchannel_ref_t* channel_ref);

/**
 * �������ӳ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 * @param timer ��ʱ��
 */
void knet_channel_ref_set_connect_timeout_timer(kchannel_ref_t* channel_ref, ktimer_t* timer);

/**
 * ��ȡ���ӳ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 * @return ��ʱ��
 */
ktimer_t* knet_channel_ref_get_connect_timeout_timer(kchannel_ref_t* channel_ref);

/**
 * ��ȡ�ܵ���ʱ���ص�����
 * @param channel_ref kchannel_ref_tʵ��
 * @return �ܵ���ʱ���ص�����
 */
ktimer_cb_t knet_channel_ref_get_timer_cb(kchannel_ref_t* channel_ref);

/**
 * ���ر��¼��Ƿ��Ѿ�������
 * @param channel_ref kchannel_ref_tʵ��
 * @retval 0 û��
 * @retval ���� �Ѿ�������
 */
int knet_channel_ref_check_close_cb_called(kchannel_ref_t* channel_ref);

/**
 * ���ùر��¼������־
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_set_close_cb_called(kchannel_ref_t* channel_ref);

/**
 * �������ճ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_ref_start_recv_timeout_timer(kchannel_ref_t* channel_ref);

/**
 * ���ٽ��ճ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_stop_recv_timeout_timer(kchannel_ref_t* channel_ref);

/**
 * �������ӳ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
int knet_channel_ref_start_connect_timeout_timer(kchannel_ref_t* channel_ref);

/**
 * �������ӳ�ʱ��ʱ��
 * @param channel_ref kchannel_ref_tʵ��
 */
void knet_channel_ref_stop_connect_timeout_timer(kchannel_ref_t* channel_ref);

#endif /* CHANNEL_REF_H */
