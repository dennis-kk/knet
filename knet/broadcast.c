/*
 * Copyright (c) 2014-2016, dennis wang
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

#include "broadcast_api.h"
#include "hash.h"
#include "channel_ref.h"
#include "misc.h"
#include "logger.h"

struct _broadcast_t {
    uint64_t domain_id; /* 域ID */
    khash_t* channels;  /* 广播域内管道引用链表 */
    klock_t* lock;      /* 锁 */
};

kbroadcast_t* knet_broadcast_create() {
    kbroadcast_t* broadcast = create(kbroadcast_t);
    verify(broadcast);
    memset(broadcast, 0, sizeof(kbroadcast_t));
    broadcast->channels = hash_create(256, 0);
    verify(broadcast->channels);
    /* 生成一个域ID */
    broadcast->domain_id = uuid_create();
    broadcast->lock      = lock_create();
    return broadcast;
}

void knet_broadcast_destroy(kbroadcast_t* broadcast) {
    khash_value_t*  value       = 0;
    kchannel_ref_t* channel_ref = 0;
    verify(broadcast);
    /* 销毁所有引用 */
    if (broadcast->channels) {
        hash_for_each_safe(broadcast->channels, value) {
            channel_ref = (kchannel_ref_t*)hash_value_get_value(value);
            verify(channel_ref);
            knet_channel_ref_decref(channel_ref);
        }
        hash_destroy(broadcast->channels);
    }
    if (broadcast->lock) {
        lock_destroy(broadcast->lock);
    }
    destroy(broadcast);
}

int knet_broadcast_join(kbroadcast_t* broadcast, kchannel_ref_t* channel_ref) {
    uint64_t uuid  = 0;
    int      error = error_ok;
    verify(broadcast);
    verify(channel_ref);
    lock_lock(broadcast->lock);
    /* 增加引用计数 */
    knet_channel_ref_incref(channel_ref);
    uuid = knet_channel_ref_get_uuid(channel_ref);
    /* 添加到广播域链表，设置链表节点 */
    error = hash_add(broadcast->channels, uuid_get_high32(uuid), channel_ref);
    if (error_ok != error) {
        lock_unlock(broadcast->lock);
        return error;
    }
    lock_unlock(broadcast->lock);
    return error_ok;
}

int knet_broadcast_leave(kbroadcast_t* broadcast, kchannel_ref_t* channel_ref) {
    uint64_t uuid  = 0;
    verify(broadcast);
    verify(channel_ref);
    verify(knet_channel_ref_check_share(channel_ref));
    uuid = knet_channel_ref_get_uuid(channel_ref);
    lock_lock(broadcast->lock);
    if (!hash_get(broadcast->channels, uuid_get_high32(uuid))) {
        lock_unlock(broadcast->lock);
        return error_broadcast_not_found;
    }
    /* 减少引用计数 */
    knet_channel_ref_decref(channel_ref);
    hash_delete(broadcast->channels, uuid_get_high32(uuid));
    lock_unlock(broadcast->lock);
    return error_ok;
}

int knet_broadcast_get_count(kbroadcast_t* broadcast) {
    int count = 0;
    verify(broadcast);
    verify(broadcast->channels);
    verify(broadcast->lock);
    lock_lock(broadcast->lock);
    count = hash_get_size(broadcast->channels);
    lock_unlock(broadcast->lock);
    return count;
}

int knet_broadcast_write(kbroadcast_t* broadcast, char* buffer, uint32_t size) {
    khash_value_t*  value       = 0;
    kchannel_ref_t* channel_ref = 0;
    int             error       = 0;
    int             count       = 0;
    verify(broadcast);
    verify(buffer);
    verify(size);
    verify(broadcast->channels);
    verify(broadcast->lock);
    lock_lock(broadcast->lock);
    hash_for_each_safe(broadcast->channels, value) {
        channel_ref = (kchannel_ref_t*)hash_value_get_value(value);
        verify(channel_ref);
        error = knet_channel_ref_write(channel_ref, buffer, size);
        if (error == error_ok) {
            count++;
        }
    }
    lock_unlock(broadcast->lock);
    return count;
}
