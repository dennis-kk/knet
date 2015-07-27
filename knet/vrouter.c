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

#include "vrouter_api.h"
#include "hash.h"
#include "misc.h"
#include "channel_ref.h"
#include "stream.h"

typedef struct _wire_t {
    kchannel_ref_t* c1; /* 起始管道 */
    kchannel_ref_t* c2; /* 目标管道 */
} wire_t;

struct _vrouter_t {
    khash_t* table; /* 查找表 */
};

/**
 * 哈希表值销毁函数
 */
void _hash_dtor(void* v);

kvrouter_t* knet_vrouter_create() {
    kvrouter_t* router = create(kvrouter_t);
    verify(router);
    memset(router, 0, sizeof(kvrouter_t));
    router->table = hash_create(1024, _hash_dtor);
    verify(router->table);
    return router;
}

void knet_vrouter_destroy(kvrouter_t* router) {
    verify(router);
    verify(router->table);
    hash_destroy(router->table);
    destroy(router);
}

int knet_vrouter_add_wire(kvrouter_t* router,  kchannel_ref_t* c1, kchannel_ref_t* c2) {
    uint32_t id    = 0;
    int      error = 0;
    wire_t*  w     = 0;
    verify(router);
    verify(c1);
    verify(c2);
    /* 取高32位自增长ID */
    id = uuid_get_high32(knet_channel_ref_get_uuid(c1));
    if (hash_get(router->table, id)) {
        /* 作为键只能出现一次 */
        return error_router_wire_exist;
    }
    w = create(wire_t);
    w->c1 = knet_channel_ref_share(c1);
    verify(w->c1);
    w->c2 = knet_channel_ref_share(c2);
    verify(w->c2);
    error = hash_add(router->table, id, w);
    if (error_ok != error) {
        goto error_return;
    }
    return error;
error_return:
    if (w) {
        if (w->c1) {
            knet_channel_ref_leave(w->c1);
        }
        if (w->c2) {
            knet_channel_ref_leave(w->c2);
        }
        destroy(w);
    }
    return error;
}

int knet_vrouter_remove_wire(kvrouter_t* router, kchannel_ref_t* c) {
    uint32_t id    = 0;
    wire_t*  w     = 0;
    int      error = 0;
    verify(router);
    verify(c);
    id = uuid_get_high32(knet_channel_ref_get_uuid(c));
    w = (wire_t*)hash_get(router->table, id);
    if (!w) {
        return error_router_wire_not_found;
    }
    error = hash_delete(router->table, id);
    if (error_ok != error) {
        verify(0);
    }
    return error;
}

int knet_vrouter_route(kvrouter_t* router, kchannel_ref_t* c, void* buffer, int size) {
    uint32_t   id = 0;
    wire_t*    w  = 0;
    kstream_t* s  = 0;
    verify(router);
    verify(c);
    verify(buffer);
    verify(size);
    id = uuid_get_high32(knet_channel_ref_get_uuid(c));
    w = (wire_t*)hash_get(router->table, id);
    if (!w) {
        return error_router_wire_not_found;
    }
    verify(w->c2);
    s = knet_channel_ref_get_stream(w->c2);
    verify(s);
    /* 发送 */
    return knet_stream_push(s, buffer, size);
}

void _hash_dtor(void* v) {
    wire_t* w = 0;
    verify(v);
    w = (wire_t*)v;
    verify(w->c1);
    verify(w->c2);
    knet_channel_ref_leave(w->c1);
    knet_channel_ref_leave(w->c2);
}
