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

bool Test_Channel_Ref_State_Acceptor_Close = false;

CASE(Test_Channel_Ref_State) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                EXPECT_TRUE(knet_channel_ref_check_state(channel, channel_state_active));
                EXPECT_FALSE(knet_channel_ref_check_balance(channel));
                knet_channel_ref_close(channel);
            } else if (e & channel_cb_event_close) {
                EXPECT_TRUE(knet_channel_ref_check_state(channel, channel_state_close));
            }
        }

        static void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                knet_channel_ref_set_cb(channel, acceptor_cb);
            } else if (e & channel_cb_event_close) {
                Test_Channel_Ref_State_Acceptor_Close = true;
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);
    knet_channel_ref_set_cb(acceptor, &holder::acceptor_cb);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);

    knet_loop_run(loop);
    EXPECT_TRUE(Test_Channel_Ref_State_Acceptor_Close);

    knet_loop_destroy(loop);
}

CASE(Test_Channel_Ref_Close_Socket_Fd_Outside) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                #ifdef WIN32
                // �������ķ�ʽ�ر�socket
                closesocket(knet_channel_ref_get_socket_fd(channel));
                #else
                close(knet_channel_ref_get_socket_fd(channel));
                #endif // WIN32
                kstream_t* s = knet_channel_ref_get_stream(channel);
                // ��Ϊ�Ѿ�������رգ�����ʧ��
                EXPECT_FALSE(error_ok == knet_stream_push(s, "123", 4));
            } else if (e & channel_cb_event_close) {
                // ��knet_loop_run�ڱ�ǿ�ƹر���
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(knet_channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(knet_channel_ref_check_state(acceptor, channel_state_accept));

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

CASE(Test_Channel_Ref_Connect_Accept_Twice) {
    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);
    EXPECT_FALSE(error_ok == knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1));

    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    EXPECT_FALSE(error_ok == knet_channel_ref_accept(acceptor, 0, 8000, 1));

    knet_loop_destroy(loop);
}

CASE(Test_Channel_Ref_Uuid) {
    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* channel = knet_loop_create_channel(loop, 1, 1024);
    EXPECT_TRUE(knet_channel_ref_get_uuid(channel));

    knet_loop_destroy(loop);
}

CASE(Test_Channel_Connect_Timeout) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {                
            } else {
                CASE_FAIL();
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

bool Test_Channel_Connect_Timeout2_Accept = 0;

CASE(Test_Channel_Connect_Timeout_Reconnect) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                if (!Test_Channel_Connect_Timeout2_Accept) {
                    kchannel_ref_t* acceptor = knet_loop_create_channel(knet_channel_ref_get_loop(channel), 1, 100);
                    knet_channel_ref_accept(acceptor, 0, 8000, 10);
                    Test_Channel_Connect_Timeout2_Accept = true;
                }
                // ����
                knet_channel_ref_reconnect(channel, 2);
            } else if (e & channel_cb_event_connect) {
                // �����ɹ����˳�loop
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {
            } else {
                CASE_FAIL();
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

CASE(Test_Channel_Connect_Timeout_Auto_Reconnect) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                if (!Test_Channel_Connect_Timeout2_Accept) {
                    kchannel_ref_t* acceptor = knet_loop_create_channel(knet_channel_ref_get_loop(channel), 1, 100);
                    knet_channel_ref_accept(acceptor, 0, 8000, 10);
                    Test_Channel_Connect_Timeout2_Accept = true;
                }
            } else if (e & channel_cb_event_connect) {
                // �����ɹ����˳�loop
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {
            } else {
                CASE_FAIL();
            }
        }
    };

    Test_Channel_Connect_Timeout2_Accept = 0;
    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    knet_channel_ref_set_auto_reconnect(connector, 1);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

CASE(Test_Channel_Idle_Timeout) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {                
            } else {
                CASE_FAIL();
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

bool case_Test_Channel_Recv_Timeout = false;

CASE(Test_Channel_Recv_Timeout) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
            } else if (e & channel_cb_event_close) {
            }
        }

        static void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                knet_channel_ref_set_cb(channel, acceptor_cb);
                knet_channel_ref_set_timeout(channel, 1);
            } else if (e & channel_cb_event_timeout) {
                case_Test_Channel_Recv_Timeout = true;
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            }
        }
    };
    kloop_t* loop = knet_loop_create();
    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, LOOP_ADDR, 8000, 1);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    knet_channel_ref_set_cb(acceptor, &holder::acceptor_cb);
    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

kchannel_ref_t* case_Test_Channel_Share_Leave_channel = 0;

CASE(Test_Channel_Share_Leave) {
    // ��ʵ�����Թ����Ƿ�ﵽҪ��ͨ��knet_channel_ref_share/knet_channel_ref_leave
    // �ڶ��̻߳�����ʹ�ã���ֻ��һ���̵߳������Ҳ�������ڹܵ����ñ��ദʹ�õ��ֲ���ͳһ����
    // �ܵ��������ڵ����

    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                // ����һ��������
                case_Test_Channel_Share_Leave_channel = knet_channel_ref_share(channel);
                // �رչܵ�
                knet_channel_ref_close(channel);
            } else if (e & channel_cb_event_close) {
                // �յ��ر��¼�, ������Ϊ�ж�һ�����ã��ܵ����ܱ�����
                knet_loop_exit(knet_channel_ref_get_loop(channel));
                // knet_loop_exit���ú�ܵ���״̬���ջ��ǻᱻɨ��һ��
                // ���ɨ���йܵ���������
            }
        }
    };

    kloop_t* loop = knet_loop_create();
    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    knet_channel_ref_connect(connector, LOOP_ADDR, 8000, 1);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(knet_channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(knet_channel_ref_check_state(acceptor, channel_state_accept));

    knet_loop_run(loop);
    // ��������
    knet_channel_ref_leave(case_Test_Channel_Share_Leave_channel);
    // ���ٹܵ�
    for (int i = 0; i < 3; i++) {
        knet_loop_run_once(loop);
    }
    EXPECT_TRUE(0 == knet_loop_get_close_channel_count(loop));
    // ʣ���3���ܵ������ﱻ����
    knet_loop_destroy(loop);
}
