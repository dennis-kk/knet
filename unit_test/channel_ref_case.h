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

CASE(Test_Channel_Ref_State) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                EXPECT_TRUE(knet_channel_ref_check_state(channel, channel_state_active));
                EXPECT_FALSE(knet_channel_ref_check_balance(channel));
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {
                EXPECT_TRUE(knet_channel_ref_check_state(channel, channel_state_close));
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(knet_channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(knet_channel_ref_check_state(acceptor, channel_state_accept));

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

CASE(Test_Channel_Ref_Close_Socket_Fd_Outside) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                #ifdef WIN32
                // 非正常的方式关闭socket
                closesocket(knet_channel_ref_get_socket_fd(channel));
                #else
                close(knet_channel_ref_get_socket_fd(channel));
                #endif // WIN32
                kstream_t* s = knet_channel_ref_get_stream(channel);
                // 因为已经被意外关闭，发送失败
                EXPECT_FALSE(error_ok == knet_stream_push(s, "123", 4));
            } else if (e & channel_cb_event_close) {
                // 在knet_loop_run内被强制关闭了
                knet_loop_exit(knet_channel_ref_get_loop(channel));
            }
        }
    };

    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);
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
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);
    EXPECT_FALSE(error_ok == knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1));

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
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);

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
                // 重连
                knet_channel_ref_reconnect(channel, 2);
            } else if (e & channel_cb_event_connect) {
                // 重连成功，退出loop
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
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);

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
                // 重连成功，退出loop
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
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

CASE(Test_Channel_Idle_Timeout) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_timeout) {
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
    knet_channel_ref_set_timeout(connector, 1); /* 设置读超时 */
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 0);

    knet_loop_run(loop);
    knet_loop_destroy(loop);
}

kchannel_ref_t* case_Test_Channel_Share_Leave_channel = 0;

CASE(Test_Channel_Share_Leave) {
    // 本实例测试功能是否达到要求，通常knet_channel_ref_share/knet_channel_ref_leave
    // 在多线程环境内使用，在只有一个线程的情况下也可以用于管道引用被多处使用但又不想统一管理
    // 管道生命周期的情况

    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                // 建立一个新引用
                case_Test_Channel_Share_Leave_channel = knet_channel_ref_share(channel);
                // 关闭管道
                knet_channel_ref_close(channel);
            } else if (e & channel_cb_event_close) {
                // 收到关闭事件, 但是因为有多一个引用，管道不能被销毁
                knet_loop_exit(knet_channel_ref_get_loop(channel));
                // knet_loop_exit调用后管道的状态最终还是会被扫描一次
                // 这次扫描中管道将被销毁
            }
        }
    };

    kloop_t* loop = knet_loop_create();
    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 1);
    knet_channel_ref_connect(connector, "127.0.0.1", 8000, 1);
    knet_channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(knet_channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(knet_channel_ref_check_state(acceptor, channel_state_accept));

    knet_loop_run(loop);
    // 销毁引用
    knet_channel_ref_leave(case_Test_Channel_Share_Leave_channel);
    // 销毁管道
    for (int i = 0; i < 3; i++) {
        knet_loop_run_once(loop);
    }
    EXPECT_TRUE(0 == knet_loop_get_close_channel_count(loop));
    // 剩余的3条管道在这里被销毁
    knet_loop_destroy(loop);
}
