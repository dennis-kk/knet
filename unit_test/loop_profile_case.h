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

int Test_Loop_Profile_i = 0;
int Test_Loop_Profile_Client_Count = 10;

CASE(Test_Loop_Profile_Dump_Stdout) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                knet_channel_ref_close(channel);                
            }
        }

        static void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_close) {
                Test_Loop_Profile_i++;
                if (Test_Loop_Profile_i == Test_Loop_Profile_Client_Count) {
                    knet_loop_exit(knet_channel_ref_get_loop(channel));
                }
            }
        }

        static void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                knet_channel_ref_set_cb(channel, &holder::client_cb);        
            }
        }
    };

    kloop_t* loop = knet_loop_create();
    kloop_profile_t* profile = knet_loop_get_profile(loop);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 128);
    knet_channel_ref_set_cb(acceptor, &holder::acceptor_cb);
    knet_channel_ref_accept(acceptor, 0, 80, 10);
    for (int i = 0; i < Test_Loop_Profile_Client_Count; i++) {
        kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 128);
        knet_channel_ref_set_cb(connector, &holder::connector_cb);
        knet_channel_ref_connect(connector, 0, 80, 0);
    }
    knet_loop_run(loop);
    EXPECT_TRUE(error_ok == knet_loop_profile_dump_stdout(profile));
    EXPECT_TRUE(Test_Loop_Profile_Client_Count == Test_Loop_Profile_i);
    knet_loop_destroy(loop);
}

CASE(Test_Loop_Profile_Dump_Stream) {
    struct holder {
        static void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_connect) {
                kloop_t* loop = knet_channel_ref_get_loop(channel);
                knet_loop_profile_dump_stream(knet_loop_get_profile(loop), knet_channel_ref_get_stream(channel));
            } else if (e & channel_cb_event_close) {
                if (Test_Loop_Profile_i == Test_Loop_Profile_Client_Count) {                                    
                    knet_loop_exit(knet_channel_ref_get_loop(channel));
                }
            }
        }

        static void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_recv) {
                char buffer[1024] = {0};
                knet_stream_pop(knet_channel_ref_get_stream(channel), buffer, sizeof(buffer));
                if (Test_Loop_Profile_i == Test_Loop_Profile_Client_Count) {
                    std::cout << buffer << std::endl;
                }
                knet_channel_ref_close(channel);
            }
        }

        static void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
            if (e & channel_cb_event_accept) {
                Test_Loop_Profile_i++;
                knet_channel_ref_set_cb(channel, &holder::client_cb);        
            }
        }
    };
    Test_Loop_Profile_i = 0;
    kloop_t* loop = knet_loop_create();
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_set_cb(acceptor, &holder::acceptor_cb);
    knet_channel_ref_accept(acceptor, 0, 80, 10);
    for (int i = 0; i < Test_Loop_Profile_Client_Count; i++) {
        kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
        knet_channel_ref_set_cb(connector, &holder::connector_cb);
        knet_channel_ref_connect(connector, 0, 80, 0);
    }
    knet_loop_run(loop);
    EXPECT_TRUE(Test_Loop_Profile_Client_Count == Test_Loop_Profile_i);
    knet_loop_destroy(loop);
}
