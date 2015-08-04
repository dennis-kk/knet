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


#include "node.h"
#include "framework.h"
#include "framework_config.h"
#include "ip_filter_api.h"
#include "node_config.h"
#include "hash.h"
#include "misc.h"
#include "channel_ref.h"
#include "stream.h"
#include "address.h"

struct _node_t {
    kframework_t*   f;                    /* 框架 */
    kip_filter_t*   black_ips;            /* 黑名单IP过滤器 */
    kip_filter_t*   white_ips;            /* 白名单IP过滤器 */
    knode_config_t* c;                    /* 配置文件 */
    khash_t*        hash_node_id;         /* 哈希表 - key: node ID, 与自己建立连接的节点 */
    khash_t*        hash_node_channel_id; /* 哈希表 - key: node channel ID, 与自己建立连接的节点 */
    krwlock_t*      rwlock_node_hash;     /* 读写锁 - 保护hash_node_id */
};

struct _node_proxy_t {
    uint32_t        type;            /* 节点类型 */
    uint32_t        id;              /* 节点ID */
    kchannel_ref_t* channel;         /* 节点管道引用 */
    knode_t*        self;            /* 本机节点 */
    uint32_t        length;          /* 本次读取长度 */
    uint32_t        heartbeat_count; /* 心跳无效次数*/
};

typedef enum _node_msg_id_e {
    node_msg_resolve_req = 1, /* 请求 - 提交身份 */
    node_msg_resolve_ack,     /* 应答 - 身份提交 */
    node_msg_heartbeat_req,   /* 请求 - 心跳 */
    node_msg_heartbeat_ack,   /* 应答 - 心跳 */
    node_msg_send,            /* 通知 - 发送节点间数据包 */
    node_msg_join,            /* 通知 - 有新节点加入集群 */
} knode_msg_id_e;

#if defined(_MSC_VER )
    #pragma pack(push)
    #pragma pack(1)
#else
    #pragma pack(1) 
#endif /* defined(_MSC_VER) */

/**
 * 协议包头
 */
typedef struct _node_header_t {
    uint32_t length; /* 消息包总长度(字节)，包括knode_header_t */
    uint32_t msg_id; /* 消息ID */
} knode_header_t;

/**
 * 协议
 */
typedef struct _node_msg_t {
    knode_header_t header; /* 消息头 */
} knode_msg_t;

/**
 * 请求 - 提交新加入节点的身份
 */
typedef struct _node_login_req_t {
    char     ip[32]; /* 监听的IP地址 */
    uint16_t port;   /* 监听的端口 */
    uint32_t type;   /* 节点类型 */
    uint32_t id;     /* 节点ID */
} knode_login_req_t;

/**
 * 应答 - 返回根节点的身份
 */
typedef struct _node_login_ack_t {
    uint32_t type; /* 节点类型 */
    uint32_t id;   /* 节点ID */
} knode_login_ack_t;

/**
 * 通知 - 其他节点的数据
 */
typedef struct _node_send_t {
    uint32_t length; /* 发送长度(字节) */
} knode_send_t;

/**
 * 通知 - 有新节点加入了集群，接收到通知的已存在节点将主动连接新加入的节点
 */
typedef struct _node_join_t {
    char     ip[32]; /* 新节点的监听IP地址 */
    uint16_t port;   /* 新节点的监听端口 */
    uint32_t type;   /* 新节点类型 */
} knode_join_t;

#if defined(_MSC_VER )
    #pragma pack(pop)
#else
    #pragma pack()
#endif /* defined(_MSC_VER) */

knode_t* knet_node_create() {
    knode_t* node = create(knode_t);
    verify(node);
    memset(node, 0, sizeof(knode_t));
    node->f = knet_framework_create();
    verify(node->f);
    node->black_ips = knet_ip_filter_create();
    verify(node->black_ips);
    node->white_ips = knet_ip_filter_create();
    verify(node->white_ips);
    node->c = knet_node_config_create(node);
    verify(node->c);
    node->rwlock_node_hash = rwlock_create();
    verify(node->rwlock_node_hash);
    /* 建立哈希表 {管道ID, 节点代理}*/
    node->hash_node_channel_id = hash_create(1024, hash_node_channel_id_dtor);
    verify(node->hash_node_channel_id);
    /* 建立哈希表 {节点ID, 节点代理}*/
    node->hash_node_id = hash_create(1024, 0);
    verify(node->hash_node_id);
    return node;
}

void knet_node_destroy(knode_t* node) {
    verify(node);
    if (node->f) {
        /* 无论是否停止，强制停止 */
        knet_framework_stop(node->f);
        knet_framework_wait_for_stop(node->f);
    }
    if (node->f) {
        knet_framework_destroy(node->f);
    }
    /* 保存黑/白名单 */
    node_save_filter_files(node);
    if (node->black_ips) {
        knet_ip_filter_destroy(node->black_ips);
    }
    if (node->white_ips) {
        knet_ip_filter_destroy(node->white_ips);
    }
    if (node->c) {
        knet_node_config_destroy(node->c);
    }
    if (node->rwlock_node_hash) {
        rwlock_destroy(node->rwlock_node_hash);
    }
    if (node->hash_node_channel_id) {
        hash_destroy(node->hash_node_channel_id);
    }
    if (node->rwlock_node_hash) {
        hash_destroy(node->hash_node_id);
    }
    destroy(node);
}

knode_config_t* knet_node_get_config(knode_t* node) {
    verify(node);
    return node->c;
}

int knet_node_start(knode_t* node) {
    kframework_config_t* config = 0;
    int                  error  = error_ok;
    verify(node);
    config = knet_framework_get_config(node->f);
    verify(config);
    /* 加载IP黑/白名单 */
    error = node_open_filter_files(node);
    if (error_ok != error) {
        return error;
    }
    if (knet_node_config_check_root(node->c)) {
        /* 建立根节点监听器 */
        error = node_root_start(node);
        log_info(
            "start root node [%s:%d]",
            knet_node_config_get_ip(node->c),
            knet_node_config_get_port(node->c));
    } else {
        /* 建立节点自身的监听器 */
        error = node_local_start(node);
        log_info(
            "start node [%s:%d], type[%d], ID[%d]",
            knet_node_config_get_ip(node->c),
            knet_node_config_get_port(node->c), knet_node_config_get_type(node->c),
            knet_node_config_get_id(node->c));
        if (error_ok != error) {
            return error;
        }
        /* 建立到根节点的连接器 */
        error = node_connect_root(node);
        log_info(
            "CONNECT root node [%s:%d]",
            knet_node_config_get_root_ip(node->c),
            knet_node_config_get_root_port(node->c));
    }
    /* 启动监控端口 */
    error = node_monitor_start(node);
    if (error_ok != error) {
        return error;
    }
    /* 启动管理端口 */
    error = node_manage_start(node);
    if (error_ok != error) {
        return error;
    }
    /* 设置工作线程数量, 默认单线程 */
    knet_framework_config_set_worker_thread_count(config,
        knet_node_config_get_worker_thread_count(node->c));
    /* 启动框架 */
    return knet_framework_start(node->f);
}

int knet_node_stop(knode_t* node) {
    verify(node);
    return knet_framework_stop(node->f);
}

void knet_node_wait_for_stop(knode_t* node) {
    verify(node);
    /* 等待框架停止 */
    knet_framework_wait_for_stop(node->f);
}

int knet_node_broadcast(knode_t* node, const void* msg, int size) {
    int            error = error_ok;
    int            ret   = error_ok;
    khash_value_t* value = 0;
    knode_proxy_t* proxy = 0;
    verify(node);
    verify(msg);
    verify(size);
    rwlock_rdlock(node->rwlock_node_hash);
    hash_for_each_safe(node->hash_node_id, value) {
        proxy = (knode_proxy_t*)hash_value_get_value(value);
        verify(proxy);
        verify(proxy->channel);
        ret = node_send(proxy->channel, msg, (uint32_t)size);
        if (error_ok != ret) { /* 只是告诉调用者发生了错误 */
            log_error("sent bytes to node failed node-ID[%d], node-type[%d]",
                proxy->id, proxy->type);
            /* 关闭节点管道 */
            knet_channel_ref_close(proxy->channel);
            error = ret;
        }
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
}

int knet_node_broadcast_by_type(knode_t* node, uint32_t type, const void* msg, int size) {
    int            error = error_ok;
    int            ret   = error_ok;
    khash_value_t* value = 0;
    knode_proxy_t* proxy = 0;
    verify(node);
    verify(msg);
    verify(size);
    verify(type);
    rwlock_rdlock(node->rwlock_node_hash);
    hash_for_each_safe(node->hash_node_id, value) {
        proxy = (knode_proxy_t*)hash_value_get_value(value);
        if (proxy->type == type) {
            verify(proxy);
            verify(proxy->channel);
            ret = node_send(proxy->channel, msg, (uint32_t)size);
            if (error_ok != ret) { /* 只是告诉调用者发生了错误 */
                log_error("sent bytes to node failed node-ID[%d], node-type[%d]",
                    proxy->id, proxy->type);
                /* 关闭节点管道 */
                knet_channel_ref_close(proxy->channel);
                error = ret;
            }
        }
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
}

int knet_node_write(knode_t* node, uint32_t id, const void* msg, int size) {
    int            error = error_ok;
    knode_proxy_t* proxy = 0;
    verify(node);
    verify(msg);
    verify(size);
    verify(id);
    rwlock_rdlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_id, id);
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    error = node_send(proxy->channel, msg, (uint32_t)size);
    if (error_ok != error) {
        log_error("sent bytes to node failed node-ID[%d], node-type[%d]",
            proxy->id, proxy->type);
        /* 关闭节点管道 */
        knet_channel_ref_close(proxy->channel);
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
error_return:
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
}

int knet_node_add_node(knode_t* node, uint32_t type, uint32_t id, kchannel_ref_t* channel) {
    int            error   = error_ok;
    knode_proxy_t* proxy   = 0;
    uint64_t       uuid    = 0;
    knet_node_cb_t node_cb = 0;
    verify(node);
    verify(type);
    verify(id);
    verify(channel);
    /* 取得管道UUID */
    uuid = knet_channel_ref_get_uuid(channel);
    rwlock_wrlock(node->rwlock_node_hash);
    if (hash_get(node->hash_node_id, id)) {
        log_error("node ID exists, ID[%D]", id);
        error = error_node_exist;
        goto error_return;
    }
    if (hash_get(node->hash_node_channel_id, uuid_get_high32(uuid))) {
        log_error("channel ID exists, CHANNEL ID[%lld]", uuid);
        error = error_node_exist;
        goto error_return;
    }
    proxy = node_proxy_create(node);
    verify(proxy);
    proxy->type    = type;
    proxy->id      = id;
    proxy->channel = channel;
    error = hash_add(node->hash_node_id, id, proxy);
    if (error_ok != error) {
        goto error_return;
    }
    error = hash_add(node->hash_node_channel_id, uuid_get_high32(uuid), proxy);
    if (error_ok != error) {
        goto error_return;
    }
    /* 增加引用计数，防止回调函数期间被销毁 */
    knet_channel_ref_incref(channel);
    rwlock_wrunlock(node->rwlock_node_hash);
    node_cb = knet_node_config_get_node_cb(node->c);
    if (node_cb) {
        /* 调用回调 - node_cb_event_join */
        proxy->length = 0;
        node_cb(proxy, channel, node_cb_event_join);
    }
    knet_channel_ref_decref(channel);
    log_info("new node established, type[%d], ID[%d]", type, id);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    if (proxy) {
        node_proxy_destroy(proxy);
    }
    return error;
}

int knet_node_remove_node_by_channel_ref(knode_t* node, kchannel_ref_t* channel) {
    knode_proxy_t* proxy     = 0;
    int            error     = error_ok;
    uint64_t       uuid      = 0;
    knet_node_cb_t node_cb   = 0;
    uint32_t       node_id   = 0;
    uint32_t       node_type = 0;
    verify(node);
    verify(channel);
    uuid = knet_channel_ref_get_uuid(channel);
    rwlock_wrlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    node_cb = knet_node_config_get_node_cb(node->c);
    if (node_cb) {
        /* 调用回调 - node_cb_event_disjoin */
        proxy->length = 0;
        node_cb(proxy, proxy->channel, node_cb_event_disjoin);
    }
    node_id   = proxy->id;
    node_type = proxy->type;
    if (error_ok != hash_delete(node->hash_node_channel_id, uuid_get_high32(uuid))) {
        error = error_node_not_found;
        goto error_return;
    }
    if (error_ok != hash_delete(node->hash_node_id, node_id)) {
        error = error_node_not_found;
        goto error_return;
    }
    rwlock_wrunlock(node->rwlock_node_hash);
    log_info("node DISCONNECT, self:type[%d], ID[%d], peer:type[%d], ID[%d]",
        knet_node_config_get_type(node->c), knet_node_config_get_id(node->c), node_type, node_id);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
}

int knet_node_remove_node_by_id(knode_t* node, uint32_t id) {
    knode_proxy_t* proxy   = 0;
    int            error   = error_ok;
    uint64_t       uuid    = 0;
    knet_node_cb_t node_cb = 0;
    uint32_t       node_id = 0;
    verify(node);
    verify(id);
    rwlock_wrlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_id, id);
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    node_cb = knet_node_config_get_node_cb(node->c);
    if (node_cb) {
        /* 调用回调 - node_cb_event_disjoin */
        proxy->length = 0;
        node_cb(proxy, proxy->channel, node_cb_event_disjoin);
    }
    node_id = proxy->id;
    uuid = knet_channel_ref_get_uuid(proxy->channel);
    if (error_ok != hash_delete(node->hash_node_channel_id, uuid_get_high32(uuid))) {
        error = error_node_not_found;
        goto error_return;
    }
    if (error_ok != hash_delete(node->hash_node_id, node_id)) {
        error = error_node_not_found;
        goto error_return;
    }
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
}

uint32_t knet_node_proxy_get_id(knode_proxy_t* proxy) {
    verify(proxy);
    return proxy->id;
}

uint32_t knet_node_proxy_get_type(knode_proxy_t* proxy) {
    verify(proxy);
    return proxy->type;
}

uint32_t knet_node_proxy_get_data_length(knode_proxy_t* proxy) {
    verify(proxy);
    return proxy->length;
}

knode_t* knet_node_proxy_get_self(knode_proxy_t* proxy) {
    verify(proxy);
    return proxy->self;
}

kframework_t* knet_node_get_framework(knode_t* node) {
    verify(node);
    return node->f;
}

kip_filter_t* knet_node_get_black_ip_filter(knode_t* node) {
    verify(node);
    return node->black_ips;
}

kip_filter_t* knet_node_get_white_ip_filter(knode_t* node) {
    verify(node);
    return node->white_ips;
}

knode_proxy_t* node_proxy_create(knode_t* self) {
    knode_proxy_t* proxy = create(knode_proxy_t);
    verify(proxy);
    memset(proxy, 0, sizeof(knode_proxy_t));
    proxy->self = self;
    return proxy;
}

void node_proxy_destroy(knode_proxy_t* proxy) {
    verify(proxy);
    if (proxy->channel) {
        knet_channel_ref_decref(proxy->channel);
    }
    destroy(proxy);
}

void hash_node_channel_id_dtor(void* param) {
    knode_proxy_t* proxy = 0;
    verify(param);
    proxy = (knode_proxy_t*)param;
    node_proxy_destroy(proxy);
}

int node_local_start(knode_t* node) {
    kframework_acceptor_config_t* acceptor = 0;
    kframework_config_t*          config   = 0;
    verify(node);
    config = knet_framework_get_config(node->f);
    verify(config);
    acceptor = knet_framework_config_new_acceptor(config);
    verify(acceptor);
    knet_framework_acceptor_config_set_local_address(acceptor,
        knet_node_config_get_ip(node->c), knet_node_config_get_port(node->c));
    framework_acceptor_config_set_user_data(acceptor, node);
    knet_framework_acceptor_config_set_backlog(acceptor, 5000);
    knet_framework_acceptor_config_set_client_heartbeat_timeout(acceptor,
        knet_node_config_get_node_channel_idle_timeout(node->c));
    knet_framework_acceptor_config_set_client_max_send_list_count(acceptor,
        knet_node_config_get_node_channel_max_send_list_count(node->c));
    knet_framework_acceptor_config_set_client_max_recv_buffer_length(acceptor,
        knet_node_config_get_node_channel_max_recv_buffer_length(node->c));
    knet_framework_acceptor_config_set_client_cb(acceptor, node_channel_cb);
    return error_ok;
}

int node_monitor_start(knode_t* node) {
    kframework_acceptor_config_t* acceptor = 0;
    kframework_config_t*          config   = 0;
    verify(node);
    config = knet_framework_get_config(node->f);
    verify(config);
    if (!knet_node_config_get_monitor_ip(node->c)[0]) {
        return error_ok;
    }
    acceptor = knet_framework_config_new_acceptor(config);
    verify(acceptor);
    knet_framework_acceptor_config_set_local_address(acceptor,
        knet_node_config_get_monitor_ip(node->c), knet_node_config_get_monitor_port(node->c));
    framework_acceptor_config_set_user_data(acceptor, node);
    knet_framework_acceptor_config_set_backlog(acceptor, 5000);
    knet_framework_acceptor_config_set_client_max_send_list_count(acceptor, 8);
    knet_framework_acceptor_config_set_client_max_recv_buffer_length(acceptor, 1024);
    knet_framework_acceptor_config_set_client_heartbeat_timeout(acceptor, 5);
    knet_framework_acceptor_config_set_client_cb(acceptor, node_monitor_channel_cb);
    return error_ok;
}

int node_manage_start(knode_t* node) {
    kframework_acceptor_config_t* acceptor = 0;
    kframework_config_t*          config   = 0;
    verify(node);
    config = knet_framework_get_config(node->f);
    verify(config);
    if (!knet_node_config_get_manage_ip(node->c)[0]) {
        return error_ok;
    }
    acceptor = knet_framework_config_new_acceptor(config);
    verify(acceptor);
    knet_framework_acceptor_config_set_local_address(acceptor,
        knet_node_config_get_manage_ip(node->c), knet_node_config_get_manage_port(node->c));
    framework_acceptor_config_set_user_data(acceptor, node);
    knet_framework_acceptor_config_set_backlog(acceptor, 5000);
    knet_framework_acceptor_config_set_client_max_send_list_count(acceptor, 64);
    knet_framework_acceptor_config_set_client_max_recv_buffer_length(acceptor, 1024 * 64);
    knet_framework_acceptor_config_set_client_heartbeat_timeout(acceptor, 5);
    knet_framework_acceptor_config_set_client_cb(acceptor, node_manage_channel_cb);
    return error_ok;
}

int node_root_start(knode_t* node) {
    return node_local_start(node);
}

int node_connect_root(knode_t* node) {
    kframework_connector_config_t* connector = 0;
    kframework_config_t*           config    = 0;
    verify(node);
    config    = knet_framework_get_config(node->f);
    connector = knet_framework_config_new_connector(config);
    verify(connector);
    knet_framework_connector_config_set_remote_address(connector,
        knet_node_config_get_root_ip(node->c), knet_node_config_get_root_port(node->c));
    framework_connector_config_set_user_data(connector, node);
    knet_framework_connector_config_set_heartbeat_timeout(connector,
        knet_node_config_get_node_channel_idle_timeout(node->c));
    knet_framework_connector_config_set_client_max_send_list_count(connector,
        knet_node_config_get_node_channel_max_send_list_count(node->c));
    knet_framework_connector_config_set_client_max_recv_buffer_length(connector,
        knet_node_config_get_node_channel_max_recv_buffer_length(node->c));
    knet_framework_connector_config_set_cb(connector, node_channel_cb);
    return error_ok;
}

int node_open_filter_files(knode_t* node) {
    int error = error_ok;
    /* 加载IP黑名单文件 */
    if (knet_node_config_get_black_ip_filter_file_path(node->c)) {
        error = knet_ip_filter_load_file(node->black_ips,
            knet_node_config_get_black_ip_filter_file_path(node->c));
        if (error_ok != error) {
            /* 错误但继续后续流程 */
            log_error("cannot open black IP filter file '%s'",
                knet_node_config_get_black_ip_filter_file_path(node->c));
        }
    }
    /* 加载IP白名单文件 */
    if (knet_node_config_get_white_ip_filter_file_path(node->c)) {
        error = knet_ip_filter_load_file(node->white_ips,
            knet_node_config_get_white_ip_filter_file_path(node->c));
        if (error_ok != error) {
            /* 错误但继续后续流程 */
            log_error("cannot open white IP filter file '%s'",
                knet_node_config_get_white_ip_filter_file_path(node->c));
        }
    }
    return error;
}

void node_save_filter_files(knode_t* node) {
    if (knet_node_config_get_black_ip_filter_auto_save_at_exit(node->c)) {
        /* 保存黑名单 */
        if (node->black_ips) {
            if (knet_node_config_get_black_ip_filter_file_path(node->c)) {
                if (error_ok != knet_ip_filter_save(node->black_ips,
                    knet_node_config_get_black_ip_filter_file_path(node->c))) {
                        log_error("save black ip filter failed, '%s'",
                            knet_node_config_get_black_ip_filter_file_path(node->c));
                }
            }
        }
    }
    if (knet_node_config_get_white_ip_filter_auto_save_at_exit(node->c)) {
        /* 保存白名单 */
        if (node->white_ips) {
            if (knet_node_config_get_white_ip_filter_file_path(node->c)) {
                if (error_ok != knet_ip_filter_save(node->white_ips,
                    knet_node_config_get_white_ip_filter_file_path(node->c))) {
                        log_error("save white ip filter failed, '%s'",
                            knet_node_config_get_white_ip_filter_file_path(node->c));
                }
            }
        }
    }
}

int on_node_connect(kchannel_ref_t* channel) {
    return node_login_req(channel);
}

int on_node_accept(kchannel_ref_t* channel) {
    kaddress_t* address = 0;
    knode_t*    node    = 0;
    node = (knode_t*)knet_channel_ref_get_user_data(channel);
    verify(node);
    address = knet_channel_ref_get_peer_address(channel);
    if (!knet_ip_filter_check(node->white_ips, address_get_ip(address))) { /* 白名单优先 */
        if (knet_ip_filter_check(node->black_ips, address_get_ip(address))) { /* 黑名单 */
            return error_node_ip_filter;
        }
    }
    return error_ok;
}

int on_node_recv(kchannel_ref_t* channel) {
    int      error = error_ok;
    uint32_t msgid = 0;
    msgid = copy_msg_id(channel);
    for (; msgid; msgid = copy_msg_id(channel)) {
        if (msgid == node_msg_resolve_req) {
            error = on_node_login_req(channel);
        } else if (msgid == node_msg_resolve_ack) {
            error = on_node_login_ack(channel);
        } else if (msgid == node_msg_join) {
            error = on_node_join(channel);
        } else if (msgid == node_msg_send) {
            error = on_node_data(channel);
        } else if (msgid == node_msg_heartbeat_req) {
            error = on_node_heartbeat_req(channel);
        } else if (msgid == node_msg_heartbeat_ack) {
            error = on_node_heartbeat_ack(channel);
        } else {
            error = error_node_invalid_msg;
        }
        /* 任何错误发生，返回错误 */
        if (error_ok != error) {
            break;
        }
    }
    return error;
}

int on_node_disjoin(kchannel_ref_t* channel) {
    knode_t* node = 0;
    verify(channel);
    node = (knode_t*)knet_channel_ref_get_user_data(channel);
    return knet_node_remove_node_by_channel_ref(node, channel);
}

int on_node_timeout(kchannel_ref_t* channel) {
    knode_t*       node  = 0;
    uint64_t       uuid  = 0;
    int            error = error_ok;
    knode_proxy_t* proxy = 0;
    node = (knode_t*)knet_channel_ref_get_user_data(channel);
    verify(node);
    uuid = knet_channel_ref_get_uuid(channel);
    rwlock_rdlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    if (proxy->heartbeat_count > 0) {
        /* 有一次心跳失效 */
        error = error_node_timeout;
    } else {
        /* 递增心跳次数 */
        proxy->heartbeat_count++;
        error = node_send_heartbeat_req(channel);
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
}

int on_node_heartbeat_req(kchannel_ref_t* channel) {
    knode_t*       node   = 0;
    uint64_t       uuid   = 0;
    int            error  = error_ok;
    knode_proxy_t* proxy  = 0;
    kstream_t*     stream = 0;
    knode_msg_t    msg;
    node = (knode_t*)knet_channel_ref_get_user_data(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    error = knet_stream_pop(stream, &msg, sizeof(knode_msg_t));
    if (error_ok != error) {
        return error;
    }
    uuid = knet_channel_ref_get_uuid(channel);
    rwlock_rdlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    error = node_send_heartbeat_ack(channel);
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
}

int on_node_heartbeat_ack(kchannel_ref_t* channel) {
    knode_t*       node   = 0;
    uint64_t       uuid   = 0;
    int            error  = error_ok;
    knode_proxy_t* proxy  = 0;
    kstream_t*     stream = 0;
    knode_msg_t    msg;
    node = (knode_t*)knet_channel_ref_get_user_data(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    error = knet_stream_pop(stream, &msg, sizeof(knode_msg_t));
    if (error_ok != error) {
        return error;
    }
    uuid = knet_channel_ref_get_uuid(channel);
    rwlock_rdlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    proxy->heartbeat_count--;
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
}

void node_channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    int error = error_ok;
    verify(channel);
    verify(e);
    if (e & channel_cb_event_connect) { /* 连接到其他节点 */
        error = on_node_connect(channel);
    } else if (e & channel_cb_event_accept) { /* 其他节点连接到自己 */
        error = on_node_accept(channel);
        /* 在node_msg_resolve_req处理函数内广播node_msg_join消息 */
    } else if (e & channel_cb_event_recv) { /* 数据包 */
        error = on_node_recv(channel);
    } else if (e & channel_cb_event_close) { /* 节点连接断开, 销毁节点数据 */
        error = on_node_disjoin(channel);
    } else if (e & channel_cb_event_timeout) { /* 心跳 */        
        error = on_node_timeout(channel);
    }
    if (error_ok != error) {
        knet_channel_ref_close(channel);
        log_error("node_channel_cb error[%d]", error);
    }
}

void node_monitor_channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    knode_t*               node       = 0;
    knode_config_t*        config     = 0;
    knet_node_monitor_cb_t monitor_cb = 0;
    verify(channel);
    verify(e);
    if (e & channel_cb_event_accept) { /* 外部监控客户端管道建立 */
        node = (knode_t*)knet_channel_ref_get_user_data(channel);
        verify(node);
        config = knet_node_get_config(node);
        verify(config);
        /* 处理监控 */
        monitor_cb = knet_node_config_get_monitor_cb(config);
        monitor_cb(node, channel);
    } else if (e & channel_cb_event_timeout) {
        /* 超时关闭 */
        knet_channel_ref_close(channel);
    }
}

void _node_manage_cmd_proc(kchannel_ref_t* channel, knode_t* node, knode_config_t* config, knet_node_manage_cb_t manage_cb) {
    kstream_t*            stream      = 0;
    char                  cmd[256]    = {0};
    int                   bytes       = sizeof(cmd) - 1;
    char*                 result      = 0;
    int                   result_size = 0;
    int                   ret_code    = manage_cb_ok;
    result_size = knet_node_config_get_manage_max_output_buffer_length(config);
    verify(result_size);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    while (error_ok == knet_stream_pop_until(stream, "\r\n", cmd, &bytes)) {
        if (result_size) {
            result = create_raw(result_size);
            verify(result);
        }
        cmd[bytes - 2] = 0; /* 防止溢出 */
        /* 处理管理命令 */
        ret_code = manage_cb(node, cmd, result, &result_size);
        if (result_size) {
            if (error_ok != knet_stream_push(stream, result, result_size)) {
                /* 发生错误，关闭管道 */
                knet_channel_ref_close(channel);
            }
        }
        if (ret_code == manage_cb_close) {
            /* 要求关闭管道 */
            knet_channel_ref_close(channel);
        }
        /* 重置属性 */
        bytes       = sizeof(cmd) - 1;
        cmd[0]      = 0;
        result[0]   = 0;
        result_size = knet_node_config_get_manage_max_output_buffer_length(config);
    }
    if (result) {
        /* 销毁结果 */
        destroy(result);
    }
}

void node_manage_channel_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    knode_t*              node      = 0;
    knode_config_t*       config    = 0;
    knet_node_manage_cb_t manage_cb = 0;
    verify(channel);
    verify(e);
    if (e & channel_cb_event_accept) { /* 外部管理客户端管道建立 */
    } else if (e & channel_cb_event_timeout) {
        /* TODO 内部比较安全的网络接口，超时且未验证完成则关闭, 是否需要身份验证？ */
    } else if (e & channel_cb_event_recv) { /* 管理命令 */
        node = (knode_t*)knet_channel_ref_get_user_data(channel);
        verify(node);
        config = knet_node_get_config(node);
        verify(config);
        manage_cb = knet_node_config_get_manage_cb(config);
        if (!manage_cb) {
            /* 不想处理，关闭管道 */
            knet_channel_ref_close(channel);
        } else {
           _node_manage_cmd_proc(channel, node, config, manage_cb);
        }
    }
}

uint32_t copy_msg_id(kchannel_ref_t* channel) {
    kstream_t* stream = 0;
    knode_msg_t msg;
    verify(channel);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    if (error_ok != knet_stream_copy(stream, &msg, sizeof(knode_msg_t))) {
        return 0;
    }
    if (msg.header.length > (uint32_t)knet_stream_available(stream)) {
        return 0;
    }
    return msg.header.msg_id;
}

int node_login_req(kchannel_ref_t* channel) {
    kstream_t*             stream      = 0;
    char                   holder[128] = {0};
    knode_t*               node        = 0;
    knode_msg_t*           msg         = (knode_msg_t*)holder;
    knode_login_req_t*     req         = (knode_login_req_t*)(holder + sizeof(knode_msg_t));
    verify(channel);
    node = (knode_t*)knet_channel_ref_get_user_data(channel);
    verify(node);
    msg->header.length = sizeof(knode_msg_t) + sizeof(knode_login_req_t);
    msg->header.msg_id = node_msg_resolve_req;
    strcpy(req->ip, knet_node_config_get_ip(node->c));
    req->port = (uint16_t)knet_node_config_get_port(node->c);
    req->id   = knet_node_config_get_id(node->c);
    req->type = knet_node_config_get_type(node->c);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    return knet_stream_push(stream, holder, msg->header.length);
}

int node_login_ack(kchannel_ref_t* channel) {
    knode_config_t*    c           = 0;
    kstream_t*         stream      = 0;
    knode_t*           node        = 0;
    char               holder[128] = {0};
    knode_msg_t*       msg         = (knode_msg_t*)holder;
    knode_login_ack_t* ack         = (knode_login_ack_t*)(holder + sizeof(knode_msg_t));
    verify(channel);
    node = knet_channel_ref_get_user_data(channel);
    verify(node);
    msg->header.length = sizeof(knode_msg_t) + sizeof(knode_login_ack_t);
    msg->header.msg_id = node_msg_resolve_ack;
    c = knet_node_get_config(node);
    ack->id   = knet_node_config_get_id(c);
    ack->type = knet_node_config_get_type(c);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    return knet_stream_push(stream, holder, msg->header.length);
}

int on_node_login_req(kchannel_ref_t* channel) {
    kstream_t*      stream  = 0;
    knode_t*        node    = 0;
    int             error   = error_ok;
    knode_proxy_t*  proxy   = 0;
    knet_node_cb_t  node_cb = 0;
    uint64_t        uuid    = 0;
    knode_login_req_t req;
    knode_msg_t msg;
    verify(channel);
    node = knet_channel_ref_get_user_data(channel);
    uuid = knet_channel_ref_get_uuid(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    error = knet_stream_pop(stream, &msg, sizeof(knode_msg_t));
    if (error_ok != error) {
        return error;
    }
    error = knet_stream_pop(stream, &req, sizeof(knode_login_req_t));
    if (error_ok != error) {
        return error;
    }
    error = knet_node_add_node(node, req.type, req.id, channel);
    if (error_ok != error) {
        return error;
    }
    error = node_login_ack(channel);
    if (error_ok != error) {
        return error;
    }
    node_cb = knet_node_config_get_node_cb(node->c);
    rwlock_rdlock(node->rwlock_node_hash);
    /* 调用节点回调 */
    if (node_cb) {
        proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
        if (proxy) {
            node_cb(proxy, channel, node_cb_event_join);
        } else {
            error = error_node_not_found;
        }
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    log_info("node login, IP[%s], port[%d], type[%d], ID[%d]", req.ip, req.port, req.type, req.id);
    /* 根节点广播join消息 */
    if (knet_node_config_check_root(node->c)) {
        return node_broadcast_join(node, req.ip, req.port, req.type, req.id);
    }
    return error;
}

int on_node_login_ack(kchannel_ref_t* channel) {
    kstream_t*     stream  = 0;
    knode_t*       node    = 0;
    int            error   = error_ok;
    knet_node_cb_t node_cb = 0;
    uint64_t       uuid    = 0;
    knode_proxy_t* proxy   = 0;
    knode_login_ack_t ack;
    knode_msg_t msg;
    verify(channel);
    node = knet_channel_ref_get_user_data(channel);
    uuid = knet_channel_ref_get_uuid(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    error = knet_stream_pop(stream, &msg, sizeof(knode_msg_t));
    if (error_ok != error) {
        return error;
    }
    error = knet_stream_pop(stream, &ack, sizeof(knode_login_ack_t));
    if (error_ok != error) {
        return error;
    }
    error = knet_node_add_node(node, ack.type, ack.id, channel);
    if (error_ok != error) {
        return error;
    }
    node_cb = knet_node_config_get_node_cb(node->c);
    rwlock_rdlock(node->rwlock_node_hash);
    /* 调用节点回调 */
    if (node_cb) {        
        proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
        if (proxy) {
            node_cb(proxy, channel, node_cb_event_join);
        } else {
            error = error_node_not_found;
        }
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    log_info("node logined, type[%d], ID[%d]", ack.type, ack.id);
    return error;
}

int on_node_join(kchannel_ref_t* channel) {
    kstream_t*                     stream    = 0;
    knode_t*                       node      = 0;
    int                            error     = error_ok;
    kframework_connector_config_t* connector = 0;
    knode_join_t join;
    knode_msg_t  msg;
    verify(channel);
    node = knet_channel_ref_get_user_data(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    error = knet_stream_pop(stream, &msg, sizeof(knode_msg_t));
    if (error_ok != error) {
        return error;
    }
    error = knet_stream_pop(stream, &join, sizeof(knode_join_t));
    if (error_ok != error) {
        return error;
    }
    if (!knet_node_config_concern(node->c, join.type)) {
        /* 不关心的节点类型 */
        return error_ok;
    }
    connector = knet_framework_config_new_connector(
        knet_framework_get_config(node->f));
    verify(connector);
    knet_framework_connector_config_set_remote_address(connector,
        join.ip, join.port);
    framework_connector_config_set_user_data(connector, node);
    knet_framework_connector_config_set_cb(connector, node_channel_cb);
    log_info("CONNECT new joined node, IP[%s], port[%d]", join.ip, join.port);
    return knet_framework_connector_start(node->f, connector);
}

int on_node_data(kchannel_ref_t* channel) {
    kstream_t*     stream  = 0;
    knode_t*       node    = 0;
    int            error   = error_ok;
    knode_proxy_t* proxy   = 0;
    uint64_t       uuid    = 0;
    knet_node_cb_t node_cb = 0;
    knode_send_t send_req;
    knode_msg_t  msg;
    verify(channel);
    uuid = knet_channel_ref_get_uuid(channel);
    node = knet_channel_ref_get_user_data(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    error = knet_stream_pop(stream, &msg, sizeof(knode_msg_t));
    if (error_ok != error) {
        return error;
    }
    error = knet_stream_pop(stream, &send_req, sizeof(knode_send_t));
    if (error_ok != error) {
        return error;
    }
    rwlock_rdlock(node->rwlock_node_hash);
    proxy = (knode_proxy_t*)hash_get(node->hash_node_channel_id, uuid_get_high32(uuid));
    if (!proxy) {
        error = error_node_not_found;
        goto error_return;
    }
    proxy->length = send_req.length;
    knet_channel_ref_incref(channel);
    rwlock_rdunlock(node->rwlock_node_hash);
    node_cb = knet_node_config_get_node_cb(node->c);
    if (node_cb) {
        node_cb(proxy, channel, node_cb_event_data);
    }
    knet_channel_ref_decref(channel);
    return error;
error_return:
    rwlock_wrunlock(node->rwlock_node_hash);
    return error;
}

int node_send(kchannel_ref_t* channel, const void* data, uint32_t size) {
    kstream_t*                     stream    = 0;
    knode_t*                       node      = 0;
    int                            error     = error_ok;
    knode_send_t send_req;
    knode_msg_t msg;
    verify(channel);
    node = knet_channel_ref_get_user_data(channel);
    verify(node);
    stream = knet_channel_ref_get_stream(channel);
    msg.header.length = sizeof(knode_msg_t) + sizeof(knode_send_t) + size;
    msg.header.msg_id = node_msg_send;
    send_req.length = size;
    error = knet_stream_push(stream, &msg, sizeof(msg));
    if (error_ok != error) {
        return error;
    }
    error = knet_stream_push(stream, &send_req, sizeof(send_req));
    if (error_ok != error) {
        return error;
    }
    return knet_stream_push(stream, data, size);
}

int node_broadcast_join(knode_t* node, const char* ip, int port, uint32_t type, uint32_t id) {
    int            error       = error_ok;
    int            ret         = error_ok;
    char           holder[128] = {0};
    khash_value_t* value       = 0;
    knode_proxy_t* proxy       = 0;
    kstream_t*     stream      = 0;
    knode_msg_t*   msg         = (knode_msg_t*)holder;
    knode_join_t*  join_req    = (knode_join_t*)(holder + sizeof(knode_msg_t));
    verify(node);
    verify(ip);
    verify(port);
    msg->header.msg_id = node_msg_join;
    msg->header.length = sizeof(knode_msg_t) + sizeof(knode_join_t);
    strcpy(join_req->ip, ip);
    join_req->port = (uint16_t)port;
    join_req->type = type;
    rwlock_rdlock(node->rwlock_node_hash);
    hash_for_each_safe(node->hash_node_id, value) {
        proxy = (knode_proxy_t*)hash_value_get_value(value);
        verify(proxy);
        if (proxy->id != id) {
            verify(proxy->channel);
            stream = knet_channel_ref_get_stream(proxy->channel);
            verify(stream);
            ret = knet_stream_push(stream, holder, msg->header.length);
            if (error_ok != ret) { /* 只是告诉调用者发生了错误 */
                error = ret;
            }
        }
    }
    rwlock_rdunlock(node->rwlock_node_hash);
    return error;
}

int node_send_heartbeat_req(kchannel_ref_t* channel) {
    char           holder[128] = {0};
    kstream_t*     stream      = 0;
    knode_msg_t*   msg         = (knode_msg_t*)holder;
    verify(channel);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    msg->header.msg_id = node_msg_heartbeat_req;
    msg->header.length = sizeof(msg);
    return knet_stream_push(stream, msg, sizeof(knode_msg_t));
}

int node_send_heartbeat_ack(kchannel_ref_t* channel) {
    char           holder[128] = {0};
    kstream_t*     stream      = 0;
    knode_msg_t*   msg         = (knode_msg_t*)holder;
    verify(channel);
    stream = knet_channel_ref_get_stream(channel);
    verify(stream);
    msg->header.msg_id = node_msg_heartbeat_ack;
    msg->header.length = sizeof(msg);
    return knet_stream_push(stream, msg, sizeof(knode_msg_t));
}
