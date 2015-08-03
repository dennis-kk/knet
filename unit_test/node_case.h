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

CASE(Test_Node_Root_Start_Stop) {
    knode_t* node = knet_node_create();
    knode_config_t* nc = knet_node_get_config(node);
    EXPECT_TRUE(error_ok == knet_node_config_set_identity(nc, 1, 1));
    EXPECT_TRUE(error_ok == knet_node_config_set_address(nc, "127.0.0.1", 12345));
    EXPECT_TRUE(error_ok == knet_node_config_set_root_address(nc, "127.0.0.1", 12345));
    EXPECT_TRUE(error_ok == knet_node_start(node));
    EXPECT_TRUE(error_ok == knet_node_stop(node));
    knet_node_destroy(node);
}

CASE(Test_Node_Start_Stop) {
    knode_t* node = knet_node_create();
    knode_config_t* nc = knet_node_get_config(node);
    EXPECT_TRUE(error_ok == knet_node_config_set_identity(nc, 1, 1));
    EXPECT_TRUE(error_ok == knet_node_config_set_address(nc, "127.0.0.1", 12345));
    EXPECT_TRUE(error_ok == knet_node_config_set_root_address(nc, "127.0.0.1", 12346));
    EXPECT_TRUE(error_ok == knet_node_start(node));
    EXPECT_TRUE(error_ok == knet_node_stop(node));
    knet_node_destroy(node);
}

bool Test_Node_Connect_Root_Join_Flag = false;
bool Test_Node_Connect_Root_DisJoin_Flag = false;
knode_t* Test_Node_Root_Node = 0;
knode_t* Test_Node_Node = 0;

CASE(Test_Node_Connect) {
    struct holder {
        static void root_node_cb(knode_proxy_t* p, kchannel_ref_t* channel, knet_node_cb_event_e e) {
            verify(channel);
            verify(p);
            if (e & node_cb_event_disjoin) {
                Test_Node_Connect_Root_DisJoin_Flag = true;
                knet_node_stop(Test_Node_Root_Node);
                knet_node_stop(Test_Node_Node);
            } else if (e & node_cb_event_join) {
                Test_Node_Connect_Root_Join_Flag = true;
            }
        }

        static void node_cb(knode_proxy_t* p, kchannel_ref_t* channel, knet_node_cb_event_e e) {
            verify(p);
            if (e & node_cb_event_join) {
                knet_channel_ref_close(channel);
            }
        }
    };

    // 建立根节点
    Test_Node_Root_Node = knet_node_create();
    knode_config_t* rnc = knet_node_get_config(Test_Node_Root_Node);
    EXPECT_TRUE(error_ok == knet_node_config_set_identity(rnc, 1, 1));
    EXPECT_TRUE(error_ok == knet_node_config_set_address(rnc, "127.0.0.1", 12345));
    EXPECT_TRUE(error_ok == knet_node_config_set_root(rnc));
    EXPECT_TRUE(error_ok == knet_node_config_set_node_cb(rnc, &holder::root_node_cb));
    EXPECT_TRUE(error_ok == knet_node_start(Test_Node_Root_Node));

    // 建立普通节点
    Test_Node_Node = knet_node_create();
    knode_config_t* nc = knet_node_get_config(Test_Node_Node);
    EXPECT_TRUE(error_ok == knet_node_config_set_identity(nc, 2, 2));
    EXPECT_TRUE(error_ok == knet_node_config_set_address(nc, "127.0.0.1", 12346));
    EXPECT_TRUE(error_ok == knet_node_config_set_root_address(nc, "127.0.0.1", 12345));
    EXPECT_TRUE(error_ok == knet_node_config_set_node_cb(nc, &holder::node_cb));
    EXPECT_TRUE(error_ok == knet_node_start(Test_Node_Node));

    knet_node_wait_for_stop(Test_Node_Node);
    knet_node_wait_for_stop(Test_Node_Root_Node);

    EXPECT_TRUE(Test_Node_Connect_Root_Join_Flag);
    EXPECT_TRUE(Test_Node_Connect_Root_DisJoin_Flag);

    knet_node_destroy(Test_Node_Root_Node);
    knet_node_destroy(Test_Node_Node);
}

#define MAX_NODE 5
knode_t* Test_Node_Node_Array[MAX_NODE] = {0};
int Test_Node_Join_Count = 0;
int MAX_JOIN_TIMES = (MAX_NODE * (MAX_NODE - 1)) / 2 + MAX_NODE;

CASE(Test_Node_Join) {
    struct holder {
        static void root_node_cb(knode_proxy_t* p, kchannel_ref_t* channel, knet_node_cb_event_e e) {
            verify(channel);
            verify(p);
            verify(e);
        }

        static void node_cb(knode_proxy_t* p, kchannel_ref_t* channel, knet_node_cb_event_e e) {
            verify(p);
            verify(channel);
            if (e & node_cb_event_join) {
                Test_Node_Join_Count++;
                if (Test_Node_Join_Count == MAX_JOIN_TIMES) {
                    knet_node_stop(Test_Node_Root_Node);
                }
            }
        }
    };

    Test_Node_Root_Node = 0;

    // 建立根节点
    Test_Node_Root_Node = knet_node_create();
    knode_config_t* rnc = knet_node_get_config(Test_Node_Root_Node);
    EXPECT_TRUE(error_ok == knet_node_config_set_identity(rnc, 1, 1));
    EXPECT_TRUE(error_ok == knet_node_config_set_address(rnc, "127.0.0.1", 12345));
    EXPECT_TRUE(error_ok == knet_node_config_set_root(rnc));
    EXPECT_TRUE(error_ok == knet_node_config_set_node_cb(rnc, &holder::root_node_cb));
    EXPECT_TRUE(error_ok == knet_node_start(Test_Node_Root_Node));

    // 建立普通节点
    for (int i = 0; i < MAX_NODE; i++) {
        knode_t* node = knet_node_create();
        knode_config_t* nc = knet_node_get_config(node);
        EXPECT_TRUE(error_ok == knet_node_config_set_identity(nc, 2, i + 2)); // 从2开始
        knet_node_config_set_concern_type(nc, 2, 0);
        EXPECT_TRUE(error_ok == knet_node_config_set_address(nc, "127.0.0.1", 12346 + i));
        EXPECT_TRUE(error_ok == knet_node_config_set_root_address(nc, "127.0.0.1", 12345));
        EXPECT_TRUE(error_ok == knet_node_config_set_node_cb(nc, &holder::node_cb));
        EXPECT_TRUE(error_ok == knet_node_start(node));
        Test_Node_Node_Array[i] = node;
    }

    knet_node_wait_for_stop(Test_Node_Root_Node);
    for (int i = 0; i < MAX_NODE; i++) {
        if (Test_Node_Node_Array[i]) {
            knet_node_stop(Test_Node_Node_Array[i]);
        }
    }
    for (int i = 0; i < MAX_NODE; i++) {
        knet_node_wait_for_stop(Test_Node_Node_Array[i]);
    }
    knet_node_destroy(Test_Node_Root_Node);
    for (int i = 0; i < MAX_NODE; i++) {
        knet_node_destroy(Test_Node_Node_Array[i]);
    }
}
