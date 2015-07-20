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

#include "helper.h"
#include "knet.h"

CASE(Test_Channel_Ref_State) {
    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                EXPECT_TRUE(channel_ref_check_state(channel, channel_state_active));
                EXPECT_FALSE(channel_ref_check_balance(channel));
                loop_exit(channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {
                EXPECT_TRUE(channel_ref_check_state(channel, channel_state_close));
            }
        }
    };

    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_t* acceptor = loop_create_channel(loop, 1, 1024);
    channel_ref_accept(acceptor, 0, 80, 1);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);
    channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(channel_ref_check_state(acceptor, channel_state_accept));

    loop_run(loop);
    loop_destroy(loop);
}

CASE(Test_Channel_Ref_Close_Socket_Fd_Outside) {
    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                #ifdef WIN32
                // 非正常的方式关闭socket
                closesocket(channel_ref_get_socket_fd(channel));
                #else
                close(channel_ref_get_socket_fd(channel));
                #endif // WIN32
                stream_t* s = channel_ref_get_stream(channel);
                // 因为已经被意外关闭，发送失败
                EXPECT_FALSE(error_ok == stream_push(s, "123", 4));
            } else if (e & channel_cb_event_close) {
                // 在loop_run内被强制关闭了
                loop_exit(channel_ref_get_loop(channel));
            }
        }
    };

    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_t* acceptor = loop_create_channel(loop, 1, 1024);
    channel_ref_accept(acceptor, 0, 80, 1);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);
    channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(channel_ref_check_state(acceptor, channel_state_accept));

    loop_run(loop);
    loop_destroy(loop);
}

CASE(Test_Channel_Ref_Connect_Accept_Twice) {
    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_t* acceptor = loop_create_channel(loop, 1, 1024);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);
    EXPECT_FALSE(error_ok == channel_ref_connect(connector, "127.0.0.1", 80, 1));

    channel_ref_accept(acceptor, 0, 80, 1);
    EXPECT_FALSE(error_ok == channel_ref_accept(acceptor, 0, 80, 1));

    loop_destroy(loop);
}

CASE(Test_Channel_Ref_Uuid) {
    loop_t* loop = loop_create();

    channel_ref_t* channel = loop_create_channel(loop, 1, 1024);
    EXPECT_TRUE(channel_ref_get_uuid(channel));

    loop_destroy(loop);
}

CASE(Test_Channel_Connect_Timeout) {
    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                loop_exit(channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {                
            } else {
                CASE_FAIL();
            }
        }
    };

    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_set_cb(connector, &holder::connector_cb);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);

    loop_run(loop);
    loop_destroy(loop);
}

bool Test_Channel_Connect_Timeout2_Accept = 0;

CASE(Test_Channel_Connect_Timeout_Reconnect) {
    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                if (!Test_Channel_Connect_Timeout2_Accept) {
                    channel_ref_t* acceptor = loop_create_channel(channel_ref_get_loop(channel), 1, 100);
                    channel_ref_accept(acceptor, 0, 80, 10);
                    Test_Channel_Connect_Timeout2_Accept = true;
                }
                // 重连
                channel_ref_reconnect(channel, 2);
            } else if (e & channel_cb_event_connect) {
                // 重连成功，退出loop
                loop_exit(channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {
            } else {
                CASE_FAIL();
            }
        }
    };

    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_set_cb(connector, &holder::connector_cb);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);

    loop_run(loop);
    loop_destroy(loop);
}

CASE(Test_Channel_Connect_Timeout_Auto_Reconnect) {
    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_connect_timeout) {
                if (!Test_Channel_Connect_Timeout2_Accept) {
                    channel_ref_t* acceptor = loop_create_channel(channel_ref_get_loop(channel), 1, 100);
                    channel_ref_accept(acceptor, 0, 80, 10);
                    Test_Channel_Connect_Timeout2_Accept = true;
                }
            } else if (e & channel_cb_event_connect) {
                // 重连成功，退出loop
                loop_exit(channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {
            } else {
                CASE_FAIL();
            }
        }
    };

    Test_Channel_Connect_Timeout2_Accept = 0;
    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_set_cb(connector, &holder::connector_cb);
    channel_ref_set_auto_reconnect(connector, 1);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);

    loop_run(loop);
    loop_destroy(loop);
}

CASE(Test_Channel_Idle_Timeout) {
    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_timeout) {
                loop_exit(channel_ref_get_loop(channel));
            } else if (e & channel_cb_event_close) {                
            } else {
                CASE_FAIL();
            }
        }
    };

    loop_t* loop = loop_create();

    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_set_cb(connector, &holder::connector_cb);
    channel_ref_set_timeout(connector, 1); /* 设置读超时 */
    channel_ref_connect(connector, "127.0.0.1", 80, 0);

    loop_run(loop);
    loop_destroy(loop);
}

// C++函数体内类静态成员作用域的问题，只有放这里了
channel_ref_t* case_Test_Channel_Share_Leave_channel = 0;

CASE(Test_Channel_Share_Leave) {
    // 本实例测试功能是否达到要求，通常channel_ref_share/channel_ref_leave
    // 在多线程环境内使用，在只有一个线程的情况下也可以用于管道引用被多处使用但又不想统一管理
    // 管道生命周期的情况

    struct holder {
        static void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                // 建立一个新引用
                case_Test_Channel_Share_Leave_channel = channel_ref_share(channel);
                // 关闭管道
                channel_ref_close(channel);
            } else if (e & channel_cb_event_close) {
                // 收到关闭事件, 但是因为有多一个引用，管道不能被销毁
                loop_exit(channel_ref_get_loop(channel));
                // loop_exit调用后管道的状态最终还是会被扫描一次
                // 这次扫描中管道将被销毁
            }
        }
    };

    loop_t* loop = loop_create();
    channel_ref_t* connector = loop_create_channel(loop, 1, 1024);
    channel_ref_t* acceptor = loop_create_channel(loop, 1, 1024);
    channel_ref_accept(acceptor, 0, 80, 1);
    channel_ref_connect(connector, "127.0.0.1", 80, 1);
    channel_ref_set_cb(connector, &holder::connector_cb);
    EXPECT_TRUE(channel_ref_check_state(connector, channel_state_connect));
    EXPECT_TRUE(channel_ref_check_state(acceptor, channel_state_accept));

    loop_run(loop);
    // 销毁引用
    channel_ref_leave(case_Test_Channel_Share_Leave_channel);
    // 销毁管道
    for (int i = 0; i < 3; i++) {
        loop_run_once(loop);
    }
    EXPECT_TRUE(0 == loop_get_close_channel_count(loop));
    // 剩余的3条管道在这里被销毁
    loop_destroy(loop);
}
