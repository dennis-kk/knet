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

#ifndef CHANNEL_REF_API_H
#define CHANNEL_REF_API_H

#include "config.h"

/**
 * @defgroup �ܵ����� �ܵ�����
 * �ܵ�����
 *
 * <pre>
 * kchannel_ref_t��Ϊkchannel_t�İ�װ���������û�͸�����˹ܵ����ڲ�ʵ�֣�ͬʱ�ṩ�����ü�������
 * �ܵ����������ڹ���.
 *
 * �ܵ���3�����ͣ�
 * 
 * 1. ������
 * 2. ������
 * 3. �ɼ��������ܵ��¹ܵ�
 *
 * �ܵ���3��״̬:
 * 
 * 1. �½��� �ս�������ȷ������Ϊ���������߼���������
 * 2. ��Ծ   �Ѿ�ȷ�����Լ��Ľ�ɫ
 * 3. �ر�   �Ѿ��رգ�����δ���٣����ü�����Ϊ��
 *
 * ��û�и��ؾ��������ڵ������(kloop_tû��ͨ��knet_loop_balancer_attach������kloop_balancer_t),
 * �����������ܵ������ڵ�ǰkloop_t�����У������ɼ��������ܵĹܵ�Ҳ����kloop_t������.
 * ���kloop_t�Ѿ����������ؾ�������������/���������ܵĹܵ����ܲ��ڵ�ǰkloop_t��
 * ���У����ؾ���������ݻ�Ծ�ܵ�������������ܵ����䵽����kloop_t���У�������Ȼ�ڵ�ǰkloop_t�����У�
 * ���ȡ���ڵ�ǰ����kloop_t���ص��������Ծ�ܵ���������.
 *
 * ���Ե��ú���knet_channel_ref_check_balanceȷ���ܵ��Ƿ񱻸��ؾ�����䣬����knet_channel_ref_check_state
 * ���ܵ���ǰ������״̬��knet_channel_ref_close�رչܵ������۴�ʱ�ܵ������ü����Ƿ�Ϊ�㣬�ܵ����׽��ֶ���
 * ���رգ����ܵ����ü���Ϊ��ʱ��kloop_t�Ż�����������.����knet_channel_ref_equal�����ж������ܵ������Ƿ�
 * ָ��ͬһ���ܵ�.
 * 
 * ����ͨ������knet_channel_ref_set_timeout���ùܵ��Ķ����г�ʱ���룩������������������Ĵ�������
 * knet_channel_ref_connectʱ���һ����������һ������ֵ�������������������ӳ�ʱ���룩���������������.
 * ����knet_channel_ref_get_socket_fd�õ��ܵ��׽��֣�����knet_channel_ref_get_uuid�ĵ��ܵ�UUID.
 * </pre>
 * @{
 */

/**
 * ���ӹܵ����ü�������������ܵ��������µ�kchannel_ref_tʵ��
 *
 * knet_channel_ref_share������ɺ󣬿����ڵ�ǰ�߳��ڷ��������߳�(kloop_t)�����еĹܵ�
 * @param channel_ref kchannel_ref_tʵ��
 * @return kchannel_ref_tʵ��
 */
extern kchannel_ref_t* knet_channel_ref_share(kchannel_ref_t* channel_ref);

/**
 * ���ٹܵ����ü�����������kchannel_ref_tʵ��
 * @param channel_ref kchannel_ref_tʵ��
 */
extern void knet_channel_ref_leave(kchannel_ref_t* channel_ref);

/**
 * ���ܵ�ת��Ϊ�����ܵ�
 *
 * ����������ܵ����ܵ������ӽ�ʹ��������ܵ���ͬ�ķ��ͻ���������������ƺͽ��ܻ�������������,
 * knet_channel_ref_accept�����ܵ������ӽ������ؾ��⣬ʵ���������ĸ�kloop_t��������ʵ�����е����
 * @param channel_ref kchannel_ref_tʵ��
 * @param ip IP
 * @param port �˿�
 * @param backlog �ȴ��������ޣ�listen())
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int knet_channel_ref_accept(kchannel_ref_t* channel_ref, const char* ip, int port, int backlog);

/**
 * ��������
 *
 * ����knet_channel_ref_connect�Ĺܵ��ᱻ���ؾ��⣬ʵ���������ĸ�kloop_t��������ʵ�����е����
 * @param channel_ref kchannel_ref_tʵ��
 * @param ip IP
 * @param port �˿�
 * @param timeout ���ӳ�ʱ���룩
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int knet_channel_ref_connect(kchannel_ref_t* channel_ref, const char* ip, int port, int timeout);

/**
 * ���·�������
 *
 * <pre>
 * ��ʱ�Ĺܵ������رգ������¹ܵ��������¹ܵ���ʹ��ԭ�йܵ������ԣ������ص��������û�ָ��
 * ���timeout����Ϊ0����ʹ��ԭ�е����ӳ�ʱ�����timeout>0��ʹ���µ����ӳ�ʱ
 * </pre>
 * @param channel_ref kchannel_ref_tʵ��
 * @param timeout ���ӳ�ʱ���룩
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int knet_channel_ref_reconnect(kchannel_ref_t* channel_ref, int timeout);

/**
 * ���ùܵ��Զ�����
 * <pre>
 * auto_reconnectΪ����ֵ�����Զ����������зǴ����Ե��¹ܵ��رգ������Զ��������û��ֶ�����
 * knet_channel_ref_close�����ᴥ���Զ�����
 * </pre>
 * @param channel_ref kchannel_ref_tʵ��
 * @param auto_reconnect �Զ�������־
 */
extern void knet_channel_ref_set_auto_reconnect(kchannel_ref_t* channel_ref, int auto_reconnect);

/**
 * ���ܵ��Ƿ������Զ�����
 * @param channel_ref kchannel_ref_tʵ��
 * @retval 0 δ����
 * @retval ���� ����
 */
extern int knet_channel_ref_check_auto_reconnect(kchannel_ref_t* channel_ref);

/**
 * ���ܵ��Ƿ���ͨ�����ؾ����������ǰ��kloop_t
 * @param channel_ref kchannel_ref_tʵ��
 * @retval 0 ����
 * @retval ��0 ��
 */
extern int knet_channel_ref_check_balance(kchannel_ref_t* channel_ref);

/**
 * ���ܵ���ǰ״̬
 * @param channel_ref kchannel_ref_tʵ��
 * @param state ��Ҫ���Ե�״̬
 * @retval 1 ��
 * @retval 0 ����
 */
extern int knet_channel_ref_check_state(kchannel_ref_t* channel_ref, knet_channel_state_e state);

/**
 * �رչܵ�
 * @param channel_ref kchannel_ref_tʵ��
 */
extern void knet_channel_ref_close(kchannel_ref_t* channel_ref);

/**
 * ���ܵ��Ƿ��Ѿ��ر�
 * @param channel_ref kchannel_ref_tʵ��
 * @retval 0 δ�ر�
 * @retval ���� �ر�
 */
extern int knet_channel_ref_check_close(kchannel_ref_t* channel_ref);

/**
 * ȡ�ùܵ��׽���
 * @param channel_ref kchannel_ref_tʵ��
 * @return �׽���
 */
extern socket_t knet_channel_ref_get_socket_fd(kchannel_ref_t* channel_ref);

/**
 * ȡ�ùܵ�������
 * @param channel_ref kchannel_ref_tʵ��
 * @return kstream_tʵ��
 */
extern kstream_t* knet_channel_ref_get_stream(kchannel_ref_t* channel_ref);

/**
 * ȡ�ùܵ����������¼�ѭ��
 * @param channel_ref kchannel_ref_tʵ��
 * @return kloop_tʵ��
 */
extern kloop_t* knet_channel_ref_get_loop(kchannel_ref_t* channel_ref);

/**
 * ���ùܵ��¼��ص�
 *
 * �¼��ص����ڹ�����kloop_tʵ�������߳��ڱ��ص�
 * @param channel_ref kchannel_ref_tʵ��
 * @param cb �ص�����
 */
extern void knet_channel_ref_set_cb(kchannel_ref_t* channel_ref, knet_channel_ref_cb_t cb);

/**
 * ���ùܵ����г�ʱ
 *
 * �ܵ����г�ʱ������������Ϊ�жϣ���timeout�����δ�пɶ����ݼȴ�����ʱ
 * @param channel_ref kchannel_ref_tʵ��
 * @param timeout ��ʱ���룩
 */
extern void knet_channel_ref_set_timeout(kchannel_ref_t* channel_ref, int timeout);

/**
 * ȡ�öԶ˵�ַ
 * @param channel_ref kchannel_ref_tʵ��
 * @return kaddress_tʵ��
 */
extern kaddress_t* knet_channel_ref_get_peer_address(kchannel_ref_t* channel_ref);

/**
 * ȡ�ñ��ص�ַ
 * @param channel_ref kchannel_ref_tʵ��
 * @return kaddress_tʵ��
 */
extern kaddress_t* knet_channel_ref_get_local_address(kchannel_ref_t* channel_ref);

/**
 * ��ȡ�ܵ�UUID
 * @param channel_ref kchannel_tʵ��
 * @return �ܵ�UUID
 */
extern uint64_t knet_channel_ref_get_uuid(kchannel_ref_t* channel_ref);

/**
 * ���������ܵ������Ƿ�ָ��ͬһ���ܵ�
 * @param a kchannel_tʵ��
 * @param b kchannel_tʵ��
 * @retval 0 ��ͬ
 * @retval ���� ��ͬ 
 */
extern int knet_channel_ref_equal(kchannel_ref_t* a, kchannel_ref_t* b);

/**
 * �����û�����ָ��
 * @param channel_ref kchannel_tʵ��
 * @param ptr �û�����ָ��
 */
extern void knet_channel_ref_set_ptr(kchannel_ref_t* channel_ref, void* ptr);

/**
 * ��ȡ�û�����ָ��
 * @param channel_ref kchannel_tʵ��
 * @return �û�����ָ��
 */
extern void* knet_channel_ref_get_ptr(kchannel_ref_t* channel_ref);

/**
 * ������ǰ�ܵ����ü���
 * @param channel_ref kchannel_ref_tʵ��
 * @return ��ǰ���ü���
 */
extern int knet_channel_ref_incref(kchannel_ref_t* channel_ref);

/**
 * �ݼ���ǰ�ܵ����ü���
 * @param channel_ref kchannel_ref_tʵ��
 * @return ��ǰ���ü���
 */
extern int knet_channel_ref_decref(kchannel_ref_t* channel_ref);

/**
 * ����Ƿ���IPV6�ܵ�
 * @param channel_ref kchannel_ref_tʵ��
 * @retvel 0 ����
 * @retval ��0 ��
 */
extern int knet_channel_ref_is_ipv6(kchannel_ref_t* channel_ref);

/** @} */

#endif /* CHANNEL_REF_API_H */
