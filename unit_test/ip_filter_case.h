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

CASE(Test_Ip_Filter_Add) {
    kip_filter_t* f = knet_ip_filter_create();
    EXPECT_TRUE(error_ok == knet_ip_filter_add(f, "192.168.0.1"));
    EXPECT_TRUE(error_ok == knet_ip_filter_add(f, "127.0.0.1"));
    EXPECT_TRUE(knet_ip_filter_check(f, "192.168.0.1"));
    EXPECT_FALSE(knet_ip_filter_check(f, "192.168.0.2"));
    knet_ip_filter_destroy(f);
}

CASE(Test_Ip_Filter_Remove) {
    kip_filter_t* f = knet_ip_filter_create();
    EXPECT_TRUE(error_ok == knet_ip_filter_add(f, "192.168.0.1"));
    EXPECT_TRUE(knet_ip_filter_check(f, "192.168.0.1"));
    EXPECT_TRUE(error_ok == knet_ip_filter_remove(f, "192.168.0.1"));
    EXPECT_FALSE(knet_ip_filter_check(f, "192.168.0.1"));
    knet_ip_filter_destroy(f);
}

CASE(Test_Ip_Filter_Load) {
    std::string path = getBinaryPath() + "/ip_filter.ipf";
    kip_filter_t* f = knet_ip_filter_create();
    EXPECT_TRUE(error_ok == knet_ip_filter_load_file(f, path.c_str()));
    EXPECT_TRUE(knet_ip_filter_check(f, "192.168.0.1"));
    EXPECT_TRUE(knet_ip_filter_check(f, "1.2.3.4"));
    knet_ip_filter_destroy(f);
}
