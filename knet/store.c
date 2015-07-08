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

#include "store.h"
#include "hash.h"

struct _store_t {
    int     worker_count;
    int     bucket_count;
    hash_t* funcs;
    hash_t* data;
    int     running;
};

store_t* store_create() {
    store_t* store = create(store_t);
    verify(store);
    memset(store, 0, sizeof(store_t));
    store->worker_count = 1;    /* 默认单线程 */
    store->bucket_count = 1024; /* 默认1024个槽位 */
    return store;
}

void store_destroy(store_t* store) {
    verify(store);
    if (store->funcs) {
        hash_destroy(store->funcs);
    }
    if (store->data) {
        hash_destroy(store->data);
    }
    destroy(store);
}

int store_start(store_t* store, const char* ip, int port) {
    verify(store);
    verify(port);
    ip;
    return 0;
}

int store_stop(store_t* store) {
    verify(store);
    return 0;
}

int store_set_worker_count(store_t* store, int worker_count) {
    verify(store);
    verify(worker_count);
    return 0;
}

int store_set_bucket_count(store_t* store, int bucket_count) {
    verify(store);
    verify(bucket_count);
    return 0;
}
