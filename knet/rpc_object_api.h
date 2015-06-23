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

#ifndef RPC_OBJECT_API_H
#define RPC_OBJECT_API_H

/* TODO ������ */

#include "config.h"

/*
 * ����set�෽������ʱȷ��krpc_object_tʵ�����ͣ�����һ��ȷ�������ٴθ���
 */

/*
 * ����RPC����
 * @return krpc_object_tʵ��
 */
extern krpc_object_t* krpc_object_create();

/*
 * ����RPC����
 * @param o krpc_object_tʵ��
 */
extern void krpc_object_destroy(krpc_object_t* o);

/*
 * ����RPC��������
 * @param o krpc_object_tʵ��
 * @param type ����
 * @retval 0 ����type����
 * @retval ���� ��type����
 */
extern int krpc_object_check_type(krpc_object_t* o, krpc_type_e type);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param i8 �з���8λ
 */
extern void krpc_number_set_i8(krpc_object_t* o, int8_t i8);

/*
 * ��ȡ�з���8λ
 * @param o krpc_object_tʵ��
 * @return �з���8λ
 */
extern int8_t krpc_number_get_i8(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param i16 �з���16λ
 */
extern void krpc_number_set_i16(krpc_object_t* o, int16_t i16);

/*
 * ��ȡ�з���16λ
 * @param o krpc_object_tʵ��
 * @return �з���16λ
 */
extern int16_t krpc_number_get_i16(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param i32 �з���32λ
 */
extern void krpc_number_set_i32(krpc_object_t* o, int32_t i32);

/*
 * ��ȡ�з���32λ
 * @param o krpc_object_tʵ��
 * @return �з���32λ
 */
extern int32_t krpc_number_get_i32(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param i64 �з���64λ
 */
extern void krpc_number_set_i64(krpc_object_t* o, int64_t i64);

/*
 * ��ȡ�з���64λ
 * @param o krpc_object_tʵ��
 * @return �з���64λ
 */
extern int64_t krpc_number_get_i64(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param ui8 �޷���8λ
 */
extern void krpc_number_set_ui8(krpc_object_t* o, uint8_t ui8);

/*
 * ��ȡ�޷���8λ
 * @param o krpc_object_tʵ��
 * @return �޷���8λ
 */
extern uint8_t krpc_number_get_ui8(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param ui16 �޷���16λ
 */
extern void krpc_number_set_ui16(krpc_object_t* o, uint16_t ui16);

/*
 * ��ȡ�޷���16λ
 * @param o krpc_object_tʵ��
 * @return �޷���16λ
 */
extern uint16_t krpc_number_get_ui16(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param ui32 �޷���32λ
 */
extern void krpc_number_set_ui32(krpc_object_t* o, uint32_t ui32);

/*
 * ��ȡ�޷���32λ
 * @param o krpc_object_tʵ��
 * @return �޷���32λ
 */
extern uint32_t krpc_number_get_ui32(krpc_object_t* o);

/*
 * ��������
 * @param o krpc_object_tʵ��
 * @param ui64 �޷���64λ
 */
extern void krpc_number_set_ui64(krpc_object_t* o, int32_t ui64);

/*
 * ��ȡ�޷���64λ
 * @param o krpc_object_tʵ��
 * @return �޷���64λ
 */
extern uint64_t krpc_number_get_ui64(krpc_object_t* o);

/*
 * �����ַ���
 * @param o krpc_object_tʵ��
 * @param s �ַ���ָ��
 */
extern void krpc_string_set(krpc_object_t* o, const char* s);

/*
 * �����ַ���
 * @param o krpc_object_tʵ��
 * @param size �ַ������ȣ�������β��
 * @param s �ַ���ָ��
 */
extern void krpc_string_set_s(krpc_object_t* o, const char* s, uint16_t size);

/*
 * ��ȡ�ַ���ָ��
 * @param o krpc_object_tʵ��
 * @return �ַ���ָ��
 */
extern const char* krpc_string_get(krpc_object_t* o);

/*
 * ��ȡ�ַ������ȣ�������β��
 * @param o krpc_object_tʵ��
 * @return �ַ�������
 */
extern uint16_t krpc_string_get_size(krpc_object_t* o);

/*
 * ���ӵ�����β������Ĭ�ϳ���Ϊ8��ÿ�ε���8
 * @param o krpc_object_tʵ��
 * @param o ����������Ԫ��
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int krpc_vector_push_back(krpc_object_t* v, krpc_object_t* o);

/*
 * ȡ������Ԫ�ظ���
 * @param o krpc_object_tʵ��
 * @return ����Ԫ�ظ���
 */
extern uint32_t krpc_vector_get_size(krpc_object_t* v);

/*
 * ������ȡ������Ԫ��
 * @param o krpc_object_tʵ��
 * @param index �±�����
 * @return ����Ԫ��
 */
extern krpc_object_t* krpc_vector_get(krpc_object_t* v, int index);

/*
 * ��������������Ԫ�أ����Ѵ���Ԫ�������پ�Ԫ��
 * @param o krpc_object_tʵ��
 * @param o ��Ԫ��
 * @param index �±�����
 * @retval error_ok �ɹ�
 * @retval ���� ʧ��
 */
extern int krpc_vector_set(krpc_object_t* v, krpc_object_t* o, int index);

#endif /* RPC_OBJECT_API_H */