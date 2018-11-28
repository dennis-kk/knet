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

#include "helper.h"
#include "knet.h"

kchannel_ref_t* Test_Stream_Push_Stream_Client1 = 0;
kchannel_ref_t* Test_Stream_Push_Stream_Client2 = 0;
kchannel_ref_t* Test_Stream_Push_Stream_Connector1 = 0;
kchannel_ref_t* Test_Stream_Push_Stream_Connector2 = 0;

CASE(Test_Stream_Push_Stream) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {            
            if (e & channel_cb_event_recv) {
                // ��һ���������յ�����������������
                if (channel == Test_Stream_Push_Stream_Connector1) {
                    kstream_t* stream1 = knet_channel_ref_get_stream(channel);
                    kstream_t* stream2 = knet_channel_ref_get_stream(Test_Stream_Push_Stream_Connector2);
                    // ����������һ���ͻ��˵�����д��ڶ�����������������client_cb�ص�
                    EXPECT_TRUE(error_ok == knet_stream_push_stream(stream1, stream2));
                }
            }
        }

        static void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_recv) {
                if (channel == Test_Stream_Push_Stream_Client2) {
                    kstream_t* stream = knet_channel_ref_get_stream(channel);
                    // �յ���knet_stream_push_stream����������
                    EXPECT_TRUE(error_ok == knet_stream_eat(stream, 4));
                    EXPECT_FALSE(knet_stream_available(stream));
                    knet_loop_exit(knet_channel_ref_get_loop(channel));
                }
            }
        }

        static void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                if (Test_Stream_Push_Stream_Client1) {
                    Test_Stream_Push_Stream_Client2 = channel;
                } else {
                    Test_Stream_Push_Stream_Client1 = channel;
                }
                knet_channel_ref_set_cb(channel, client_cb);
                // �����ͻ��˶��Ѿ����ӳɹ�
                if (Test_Stream_Push_Stream_Client1 && Test_Stream_Push_Stream_Client2) {
                    // ���һ���ͻ���д��4���ֽ�
                    kstream_t* stream = knet_channel_ref_get_stream(Test_Stream_Push_Stream_Client1);
                    EXPECT_TRUE(error_ok == knet_stream_push(stream, "123", 4));
                }
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    Test_Stream_Push_Stream_Connector1 = knet_loop_create_channel(loop, 1, 1024);
    Test_Stream_Push_Stream_Connector2 = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(acceptor, &holder::acceptor_cb);
    knet_channel_ref_accept(acceptor, LOOP_ADDR, 8000, 1);
    // ���������ͻ���
    // ���õ�һ���������¼��ص�
    knet_channel_ref_set_cb(Test_Stream_Push_Stream_Connector1, &holder::connector_cb);
    knet_channel_ref_connect(Test_Stream_Push_Stream_Connector1, LOOP_ADDR, 8000, 1);
    knet_channel_ref_connect(Test_Stream_Push_Stream_Connector2, LOOP_ADDR, 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

kchannel_ref_t* Test_Stream_Copy_Stream_Client1 = 0;
kchannel_ref_t* Test_Stream_Copy_Stream_Client2 = 0;
kchannel_ref_t* Test_Stream_Copy_Stream_Connector1 = 0;
kchannel_ref_t* Test_Stream_Copy_Stream_Connector2 = 0;

CASE(Test_Stream_Copy_Stream) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {            
            if (e & channel_cb_event_recv) {
                // ��һ���������յ�����������������
                if (channel == Test_Stream_Copy_Stream_Connector1) {
                    kstream_t* stream1 = knet_channel_ref_get_stream(channel);
                    kstream_t* stream2 = knet_channel_ref_get_stream(Test_Stream_Copy_Stream_Connector2);
                    // ����������һ���ͻ��˵�����д��ڶ�����������������client_cb�ص�
                    EXPECT_TRUE(error_ok == knet_stream_copy_stream(stream1, stream2));
                    EXPECT_TRUE(4 == knet_stream_available(stream1));
                }
            }
        }

        static void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_recv) {
                if (channel == Test_Stream_Copy_Stream_Client2) {
                    kstream_t* stream = knet_channel_ref_get_stream(channel);
                    // �յ���knet_stream_copy_stream����������
                    EXPECT_TRUE(error_ok == knet_stream_eat(stream, 4));
                    EXPECT_FALSE(knet_stream_available(stream));
                    knet_loop_exit(knet_channel_ref_get_loop(channel));
                }
            }
        }

        static void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                if (Test_Stream_Copy_Stream_Client1) {
                    Test_Stream_Copy_Stream_Client2 = channel;
                } else {
                    Test_Stream_Copy_Stream_Client1 = channel;
                }
                knet_channel_ref_set_cb(channel, client_cb);
                // �����ͻ��˶��Ѿ����ӳɹ�
                if (Test_Stream_Copy_Stream_Client1 && Test_Stream_Copy_Stream_Client2) {
                    // ���һ���ͻ���д��4���ֽ�
                    kstream_t* stream = knet_channel_ref_get_stream(Test_Stream_Copy_Stream_Client1);
                    EXPECT_TRUE(error_ok == knet_stream_push(stream, "123", 4));
                }
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    Test_Stream_Copy_Stream_Connector1 = knet_loop_create_channel(loop, 1, 1024);
    Test_Stream_Copy_Stream_Connector2 = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(acceptor, &holder::acceptor_cb);
    knet_channel_ref_accept(acceptor, LOOP_ADDR, 8000, 1);

    // ���������ͻ���
    // ���õ�һ���������¼��ص�
    knet_channel_ref_set_cb(Test_Stream_Copy_Stream_Connector1, &holder::connector_cb);
    knet_channel_ref_connect(Test_Stream_Copy_Stream_Connector1, LOOP_ADDR, 8000, 1);
    knet_channel_ref_connect(Test_Stream_Copy_Stream_Connector2, LOOP_ADDR, 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}
