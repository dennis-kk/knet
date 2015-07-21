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

CASE(Test_Address) {
    kloop_t* loop = knet_loop_create();

    kchannel_ref_t* channel = knet_loop_create_channel(loop, 0, 1024);
    kaddress_t* peer = knet_channel_ref_get_peer_address(channel);
    kaddress_t* local = knet_channel_ref_get_local_address(channel);
    // 未建立连接
    EXPECT_TRUE(std::string("0.0.0.0") == address_get_ip(peer));
    EXPECT_FALSE(address_get_port(peer));
    EXPECT_TRUE_OUTPUT(std::string("0.0.0.0") == address_get_ip(local), address_get_ip(local));
    EXPECT_FALSE(address_get_port(local));

    knet_channel_ref_close(channel); /* 未建立起连接，需要手动销毁 */

    kchannel_ref_t* connector = knet_loop_create_channel(loop, 1, 1024);
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 1, 1024);
    knet_channel_ref_accept(acceptor, 0, 80, 1);
    knet_channel_ref_connect(connector, "127.0.0.1", 80, 1);
    knet_loop_run_once(loop);

    peer = knet_channel_ref_get_peer_address(connector);
    local = knet_channel_ref_get_local_address(connector);
    EXPECT_TRUE(std::string("0.0.0.0") != address_get_ip(peer));
    EXPECT_TRUE(80 == address_get_port(peer));
    EXPECT_TRUE(std::string("0.0.0.0") != address_get_ip(local));
    EXPECT_TRUE(0 != address_get_port(local));

    knet_loop_destroy(loop);
}
