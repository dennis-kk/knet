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

#ifndef LOOP_API_H
#define LOOP_API_H

#include "config.h"

/**
 * @defgroup loop �¼�ѭ��
 * �����¼�ѭ��
 *
 * <pre>
 * �����¼�API����Ϊ������ͬ����ϵͳ����ѡȡ���İ�װ�������˲�ͬƽ̨�ľ���ʵ�֣�
 * Ϊ���ṩͳһ�ĵ��ýӿ�.
 *
 * �ܵ�����kchannel_ref_tͨ������knet_loop_create_channel��knet_loop_create_channel_exist_socket_fd
 * ������knet_loop_run�������¼�ѭ�����ȴ�����knet_loop_exit�˳���������ֶ�����knet_loop_run_once����һ���¼�
 * ѭ���Լ�����ѭ���ĵ���Ƶ��.
 *
 * ÿ��kloop_t�ڶ�ά���˻�Ծ�ܵ����ѹر�(δ����)�ܵ���˫����������ͨ��knet_loop_get_active_channel_count
 * ��knet_loop_get_close_channel_count��ȡ�þ�������.
 *
 * �ڴ����ܵ�ʱ��Ҫע��������Ҫ�����ò�����
 *
 * 1. max_send_list_len �����������Ԫ�ظ���
 * 2. recv_ring_len     ���ܻ�������󳤶�
 *
 * ͨ����ܵ��ڷ�������(stream_push_ϵ��)���ܵ��᳢��ֱ�ӷ��ͣ���������Ҫ���͵����ݣ������Ϊĳ��ԭ��
 * ���²���ֱ�ӷ��ͣ����ݻᱻ�����ڷ��������ڵȴ����ʵ�ʱ�����ͣ������������ĳ��ȴﵽ���ޣ��ܵ��ᱻ�ر�.
 * ͬ�������ܻ���������׽����ڽ����ݶ�ȡ�����������һֱ����kstream_t��ȡ���ݣ���ô����ᱻд�����ܵ�Ҳ
 * �ᱻ�ر�.
 *
 * </pre>
 * @{
 */

/**
 * ����һ���¼�ѭ��
 * @return kloop_tʵ��
 */
extern kloop_t* knet_loop_create();

/**
 * �����¼�ѭ��
 * �¼�ѭ���ڵ����йܵ�Ҳ�ᱻ����
 * @param loop kloop_tʵ��
 */
extern void knet_loop_destroy(kloop_t* loop);

/**
 * �����ܵ�
 * @param loop kloop_tʵ��
 * @param max_send_list_len ���ͻ���������󳤶�
 * @param recv_ring_len ���ܻ��λ�������󳤶�
 * @return kchannel_ref_tʵ��
 */
extern kchannel_ref_t* knet_loop_create_channel(kloop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len);

/**
 * �����ܵ�
 * @param loop kloop_tʵ��
 * @param max_send_list_len ���ͻ���������󳤶�
 * @param recv_ring_len ���ܻ��λ�������󳤶�
 * @return kchannel_ref_tʵ��
 */
extern kchannel_ref_t* knet_loop_create_channel6(kloop_t* loop, uint32_t max_send_list_len, uint32_t recv_ring_len);

/**
 * ʹ���Ѵ��ڵ��׽��ִ����ܵ�
 * @param loop kloop_tʵ��
 * @param socket_fd �׽���
 * @param max_send_list_len ���ͻ���������󳤶�
 * @param recv_ring_len ���ܻ��λ�������󳤶�
 * @return kchannel_ref_tʵ��
 */
extern kchannel_ref_t* knet_loop_create_channel_exist_socket_fd(kloop_t* loop, socket_t socket_fd,
    uint32_t max_send_list_len, uint32_t recv_ring_len);

/**
 * ʹ���Ѵ��ڵ��׽��ִ����ܵ�
 * @param loop kloop_tʵ��
 * @param socket_fd �׽���
 * @param max_send_list_len ���ͻ���������󳤶�
 * @param recv_ring_len ���ܻ��λ�������󳤶�
 * @return kchannel_ref_tʵ��
 */
extern kchannel_ref_t* knet_loop_create_channel_exist_socket_fd6(kloop_t* loop, socket_t socket_fd,
    uint32_t max_send_list_len, uint32_t recv_ring_len);

/**
 * ����һ���¼�ѭ��
 * kloop_t�����̰߳�ȫ�ģ������ڶ���߳���ͬʱ��ͬһ��kloop_tʵ������knet_loop_run_once
 * @param loop kloop_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int knet_loop_run_once(kloop_t* loop);

/**
 * �����¼�ѭ��ֱ������knet_loop_exit()
 * kloop_t�����̰߳�ȫ�ģ������ڶ���߳���ͬʱ��ͬһ��kloop_tʵ������knet_loop_run
 * @param loop kloop_tʵ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int knet_loop_run(kloop_t* loop);

/**
 * �˳�����knet_loop_run()
 * @param loop kloop_tʵ��
 */
extern void knet_loop_exit(kloop_t* loop);

/**
 * ��ȡ��Ծ�ܵ�����
 * @param loop kloop_tʵ��
 * @return ��Ծ�ܵ�����
 */
extern int knet_loop_get_active_channel_count(kloop_t* loop);

/**
 * ��ȡ�ѹرչܵ�����
 * @param loop kloop_tʵ��
 * @return �رչܵ�����
 */
extern int knet_loop_get_close_channel_count(kloop_t* loop);

/**
 * ȡ��ͳ����
 * @param loop kloop_tʵ��
 * @return kloop_profile_tʵ��
 */
extern kloop_profile_t* knet_loop_get_profile(kloop_t* loop);

/** @} */

#endif /* LOOP_API_H */
