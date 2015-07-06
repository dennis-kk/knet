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

#include "broadcast_api.h"
#include "list.h"
#include "channel_ref.h"
#include "misc.h"

struct _broadcast_t {
    uint64_t domain_id; /* 域ID */
    dlist_t* channels;  /* 广播域内管道引用链表 */
    lock_t*  lock;      /* 锁 */
};

broadcast_t* broadcast_create() {
    broadcast_t* broadcast = create(broadcast_t);
    verify(broadcast);
    memset(broadcast, 0, sizeof(broadcast_t));
    broadcast->channels = dlist_create();
    if (!broadcast->channels) {
        broadcast_destroy(broadcast);
        return 0;
    }
    if (!broadcast->channels) {
        broadcast_destroy(broadcast);
        return 0;
    }
    /* 生成一个域ID */
    broadcast->domain_id = uuid_create();
    broadcast->lock      = lock_create();
    return broadcast;
}

void broadcast_destroy(broadcast_t* broadcast) {
    dlist_node_t*  node        = 0;
    dlist_node_t*  temp        = 0;
    channel_ref_t* channel_ref = 0;
    verify(broadcast);
    /* 销毁所有引用 */
    if (broadcast->channels) {
        dlist_for_each_safe(broadcast->channels, node, temp) {
            channel_ref = (channel_ref_t*)dlist_node_get_data(node);
            verify(channel_ref);
            broadcast_leave(broadcast, channel_ref);
        }
    }
    if (broadcast->lock) {
        lock_destroy(broadcast->lock);
    }
    destroy(broadcast);
}

channel_ref_t* broadcast_join(broadcast_t* broadcast, channel_ref_t* channel_ref) {
    dlist_node_t* node = 0;
    /* 创建新的引用 */
    channel_ref_t* channel_shared = 0;
    verify(broadcast);
    verify(channel_ref);
    channel_shared = channel_ref_share(channel_ref);
    verify(channel_shared);
    lock_lock(broadcast->lock);
    /* 添加到广播域链表，设置链表节点（快速删除） */
    node = dlist_add_tail_node(broadcast->channels, channel_shared);
    lock_unlock(broadcast->lock);
    channel_ref_set_domain_node(channel_shared, node);
    channel_ref_set_domain_id(channel_shared, broadcast->domain_id);
    return channel_shared;
}

int broadcast_leave(broadcast_t* broadcast, channel_ref_t* channel_ref) {
    dlist_node_t* node = 0;
    verify(broadcast);
    verify(channel_ref);
    verify(channel_ref_check_share(channel_ref));
    node = channel_ref_get_domain_node(channel_ref);
    verify(node);
    if (broadcast->domain_id != channel_ref_get_domain_id(channel_ref)) {
        return error_not_correct_domain;
    }
    lock_lock(broadcast->lock);
    dlist_delete(broadcast->channels, node);
    lock_unlock(broadcast->lock);
    /* 销毁引用 */
    channel_ref_leave(channel_ref);
    return error_ok;
}

int broadcast_get_count(broadcast_t* broadcast) {
    int count = 0;
    verify(broadcast);
    verify(broadcast->channels);
    verify(broadcast->lock);
    lock_lock(broadcast->lock);
    count = dlist_get_count(broadcast->channels);
    lock_unlock(broadcast->lock);
    return count;
}

int broadcast_write(broadcast_t* broadcast, char* buffer, uint32_t size) {
    dlist_node_t*  node        = 0;
    dlist_node_t*  temp        = 0;
    channel_ref_t* channel_ref = 0;
    int            error       = 0;
    int            count       = 0;
    verify(broadcast);
    verify(broadcast->channels);
    verify(broadcast->lock);
    verify(buffer);
    verify(size);
    lock_lock(broadcast->lock);
    dlist_for_each_safe(broadcast->channels, node, temp) {
        channel_ref = (channel_ref_t*)dlist_node_get_data(node);
        error = channel_ref_write(channel_ref, buffer, size);
        if (error != error_ok) {
            /* 销毁发送失败的管道 */
            broadcast_leave(broadcast, channel_ref);
        } else {
            count++;
        }
    }
    lock_unlock(broadcast->lock);
    return count;
}
