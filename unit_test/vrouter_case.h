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

CASE(Test_Vrouter_Add_Wire) {
    kloop_t* l = knet_loop_create();
    kchannel_ref_t* c1 = knet_loop_create_channel(l, 0, 0);
    kchannel_ref_t* c2 = knet_loop_create_channel(l, 0, 0);
    kvrouter_t* r = knet_vrouter_create();
    EXPECT_TRUE(error_ok == knet_vrouter_add_wire(r, c1, c2));
    EXPECT_FALSE(error_ok == knet_vrouter_add_wire(r, c1, c2));
    knet_vrouter_destroy(r);
    knet_loop_destroy(l);
}

CASE(Test_Vrouter_Remove_Wire) {
    kloop_t* l = knet_loop_create();
    kchannel_ref_t* c1 = knet_loop_create_channel(l, 0, 0);
    kchannel_ref_t* c2 = knet_loop_create_channel(l, 0, 0);
    kvrouter_t* r = knet_vrouter_create();
    EXPECT_TRUE(error_ok == knet_vrouter_add_wire(r, c1, c2));
    EXPECT_TRUE(error_ok == knet_vrouter_remove_wire(r, c1));
    EXPECT_FALSE(error_ok == knet_vrouter_remove_wire(r, c1));
    EXPECT_FALSE(error_ok == knet_vrouter_remove_wire(r, c2));
    knet_vrouter_destroy(r);
    knet_loop_destroy(l);
}

CASE(Test_Vrouter_Route) {
    kloop_t* l = knet_loop_create();
    kchannel_ref_t* c1 = knet_loop_create_channel(l, 0, 0);
    kchannel_ref_t* c2 = knet_loop_create_channel(l, 0, 0);
    kvrouter_t* r = knet_vrouter_create();
    EXPECT_TRUE(error_ok == knet_vrouter_add_wire(r, c1, c2));
    EXPECT_FALSE(error_ok == knet_vrouter_route(r, c1, "123", 4));
    knet_vrouter_destroy(r);
    knet_loop_destroy(l);
}
