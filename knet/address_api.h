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

#ifndef ADDRESS_API_H
#define ADDRESS_API_H

#include "config.h"

/**
 * @defgroup address ��ַ
 * ��ַ
 *
 * <pre>
 * ��ַ�ӿ�ͨ��knet_channel_ref_get_local_address��knet_channel_ref_get_peer_address
 * ��ȡ���ػ�Զ˵ĵ�ַ��δ�������ӵĹܵ�Ҳ���Ի�ȡ��ַ������ȡ�ĵ�ַ����Ч��.
 * </pre>
 * @sa knet_channel_ref_get_local_address
 * @sa knet_channel_ref_get_peer_address
 * @{
 */

/**
 * ȡ��IP
 * @param address kaddress_tʵ��
 * @retval ��Ч��ָ�� IP�ַ���
 * @retval 0 �ܵ�����δ����
 */
FuncExport const char* address_get_ip(kaddress_t* address);

/**
 * ȡ��port
 * @param address kaddress_tʵ��
 * @retval ��Ч�Ķ˿ں� �˿ں�
 * @retval 0 �ܵ�����δ����
 */
FuncExport int address_get_port(kaddress_t* address);

/**
 * �����Ƿ����
 * @param address kaddress_tʵ��
 * @param ip IP
 * @param port �˿�
 * @retval 0 ���
 * @retval ���� �����
 */
FuncExport int address_equal(kaddress_t* address, const char* ip, int port);

/** @} */

#endif /* ADDRESS_API_H */
