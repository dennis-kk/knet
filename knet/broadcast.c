#include "broadcast_api.h"
#include "list.h"
#include "channel_ref.h"
#include "misc.h"

struct _broadcast_t {
    uint64_t domain_id;
    dlist_t* channels;
    lock_t*  lock;
};

broadcast_t* broadcast_create() {
    broadcast_t* broadcast = create(broadcast_t);
    assert(broadcast);
    broadcast->channels = dlist_create();
    assert(broadcast->channels);
    /* 生成一个域ID */
    broadcast->domain_id = gen_domain_uuid();
    broadcast->lock      = lock_create();
    return broadcast;
}

void broadcast_destroy(broadcast_t* broadcast) {
    dlist_node_t*  node        = 0;
    dlist_node_t*  temp        = 0;
    channel_ref_t* channel_ref = 0;
    assert(broadcast);
    /* 销毁所有引用 */
    dlist_for_each_safe(broadcast->channels, node, temp) {
        channel_ref = (channel_ref_t*)dlist_node_get_data(node);
        broadcast_leave(broadcast, channel_ref);
    }
    lock_destroy(broadcast->lock);
    destroy(broadcast);
}

channel_ref_t* broadcast_join(broadcast_t* broadcast, channel_ref_t* channel_ref) {
    dlist_node_t* node = 0;
    /* 创建新的引用 */
    channel_ref_t* channel_shared = channel_ref_share(channel_ref);
    assert(channel_shared);
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
    assert(broadcast);
    assert(channel_ref);
    assert(channel_ref_check_share(channel_ref));
    if (broadcast->domain_id != channel_ref_get_domain_id(channel_ref)) {
        return error_not_correct_domain;
    }
    node = channel_ref_get_domain_node(channel_ref);
    assert(node);
    lock_lock(broadcast->lock);
    dlist_delete(broadcast->channels, node);
    lock_unlock(broadcast->lock);
    /* 销毁引用 */
    channel_ref_leave(channel_ref);
    return error_ok;
}

int broadcast_get_count(broadcast_t* broadcast) {
    int count;
    assert(broadcast);
    assert(broadcast->channels);
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
    assert(broadcast);
    assert(broadcast->channels);
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
