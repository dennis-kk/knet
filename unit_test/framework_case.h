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

kframework_t* Test_Framework_Framework = 0;
bool Test_Framework_Echo = false;
bool Test_Framework_Accept = false;

CASE(Test_Framework) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                kstream_t* stream = knet_channel_ref_get_stream(channel);
                knet_stream_push(stream, "1234", 5);
            } else if (e & channel_cb_event_recv) {
                Test_Framework_Echo = true;
                knet_channel_ref_close(channel);
                knet_framework_stop(Test_Framework_Framework);
            }
        }

        static void channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                Test_Framework_Accept = true;
            }
            if (e & channel_cb_event_recv) {
                kstream_t* stream = knet_channel_ref_get_stream(channel);
                knet_stream_push(stream, "1234", 5);
            }
        }
    };

    Test_Framework_Framework = knet_framework_create();
    kframework_config_t* c = knet_framework_get_config(Test_Framework_Framework);
    kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, 0, 8000);
    knet_framework_acceptor_config_set_client_cb(ac, &holder::channel_cb);
    knet_framework_config_set_worker_thread_count(c, 4);

    // 模拟一个外部连接器
    kframework_connector_config_t* cc = knet_framework_config_new_connector(c);
    knet_framework_connector_config_set_remote_address(cc, "127.0.0.1", 8000);
    knet_framework_connector_config_set_cb(cc, &holder::connector_cb);

    // 启动框架
    EXPECT_TRUE(error_ok == knet_framework_start(Test_Framework_Framework));

    knet_framework_wait_for_stop(Test_Framework_Framework);
    knet_framework_destroy(Test_Framework_Framework);

    EXPECT_TRUE(Test_Framework_Echo);
    EXPECT_TRUE(Test_Framework_Accept);
}

CASE(Test_Framework_Async) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                kstream_t* stream = knet_channel_ref_get_stream(channel);
                knet_stream_push(stream, "1234", 5);
            } else if (e & channel_cb_event_recv) {
                Test_Framework_Echo = true;
                knet_channel_ref_close(channel);
                knet_framework_stop(Test_Framework_Framework);
            }
        }

        static void channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                Test_Framework_Accept = true;
            }
            if (e & channel_cb_event_recv) {
                kstream_t* stream = knet_channel_ref_get_stream(channel);
                knet_stream_push(stream, "1234", 5);
            }
        }
    };

    Test_Framework_Framework = knet_framework_create();
    kframework_config_t* c = knet_framework_get_config(Test_Framework_Framework);
    knet_framework_config_set_worker_thread_count(c, 4);
    // 启动框架
    EXPECT_TRUE(error_ok == knet_framework_start(Test_Framework_Framework));

    // 框架启动后，建立一个监听器
    kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, 0, 8000);
    knet_framework_acceptor_config_set_client_cb(ac, &holder::channel_cb);
    knet_framework_acceptor_start(Test_Framework_Framework, ac);

    // 框架启动后，模拟一个外部连接器
    kframework_connector_config_t* cc = knet_framework_config_new_connector(c);
    knet_framework_connector_config_set_remote_address(cc, "127.0.0.1", 8000);
    knet_framework_connector_config_set_cb(cc, &holder::connector_cb);
    knet_framework_connector_start(Test_Framework_Framework, cc);

    knet_framework_wait_for_stop(Test_Framework_Framework);
    knet_framework_destroy(Test_Framework_Framework);

    EXPECT_TRUE(Test_Framework_Echo);
    EXPECT_TRUE(Test_Framework_Accept);
}

CASE(Test_Framework_Start_Fail) {
    kframework_t* f = knet_framework_create();
    kframework_config_t* c = knet_framework_get_config(f);
    kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, "128.0.0.1", 8000);
    EXPECT_FALSE(error_ok == knet_framework_start(f));
    knet_framework_destroy(f);

    f = knet_framework_create();
    c = knet_framework_get_config(f);
    ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, "128.0.0.1", 8000);
    EXPECT_FALSE(error_ok == knet_framework_start_wait(f));
    knet_framework_destroy(f);

    f = knet_framework_create();
    c = knet_framework_get_config(f);
    ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, "128.0.0.1", 8000);
    EXPECT_FALSE(error_ok == knet_framework_start_wait_destroy(f));
}

CASE(Test_Framework_Timer) {
    struct holder {
        static void channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            EXPECT_TRUE(channel);
            if (e & channel_cb_event_accept) {
                ktimer_t* timer = knet_framework_create_worker_timer(Test_Framework_Framework);
                ktimer_start(timer, &holder::timer_cb, 0, 1000);
            }            
        }

        static void timer_cb(ktimer_t* timer, void* param) {
            EXPECT_TRUE(timer);
            EXPECT_FALSE(param);
            knet_framework_stop(Test_Framework_Framework);
        }
    };
    Test_Framework_Framework = knet_framework_create();
    kframework_config_t* c = knet_framework_get_config(Test_Framework_Framework);
    kframework_acceptor_config_t* ac = knet_framework_config_new_acceptor(c);
    knet_framework_acceptor_config_set_local_address(ac, 0, 8000);
    knet_framework_acceptor_config_set_client_cb(ac, &holder::channel_cb);
    knet_framework_config_set_worker_thread_count(c, 4);

    // 模拟一个外部连接器
    kframework_connector_config_t* cc = knet_framework_config_new_connector(c);
    knet_framework_connector_config_set_remote_address(cc, "127.0.0.1", 8000);
    knet_framework_connector_config_set_cb(cc, 0);

    // 启动框架
    EXPECT_TRUE(error_ok == knet_framework_start(Test_Framework_Framework));

    knet_framework_wait_for_stop_destroy(Test_Framework_Framework);
}
