#include "example_config.h"

#if COMPILE_MULTI_CONNECTOR

#include "knet.h"

/*
 单个loop_t实例, 多个连接器，一个监听者
 */

#define MAX_CONNECTOR 200  /* 连接器启动数量 */
int connector_count = MAX_CONNECTOR; /* 当前连接器数量 */

/* 客户端 - 连接器回调 */
void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[32] = {0};
    char* hello = "hello world";
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_connect) { /* 连接成功 */
        /* 写入 */
        stream_push(stream, hello, 12);
    } else if (e & channel_cb_event_recv) {
        /* echo数据读取 */
        stream_pop(stream, buffer, sizeof(buffer));
        /* 关闭 */
        channel_ref_close(channel);
    } else if (e & channel_cb_event_connect_timeout) {
        /* 关闭 */
        channel_ref_close(channel);
        printf("connector close: timeout\n");
    }
}

/* 服务端 - 客户端回调 */
void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[32] = {0};
    address_t* peer_address = 0;
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        /* 读取 */
        stream_pop(stream, buffer, sizeof(buffer));
        /* 不论是否读取完整， 写入12字节 */
        stream_push(stream, buffer, 12);
    } else if (e & channel_cb_event_close) {
        peer_address = channel_ref_get_peer_address(channel);
        printf("peer close: %s, %d, %d\n", address_get_ip(peer_address),
            address_get_port(peer_address), connector_count);
        /* 对端关闭 */
        connector_count--;
        if (connector_count == 0) { /* 全部关闭 */
            /* 退出 */
            loop_exit(channel_ref_get_loop(channel));
        }
    }
}

/* 监听者回调 */
void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    if (e & channel_cb_event_accept) { /* 新连接 */
        /* 设置回调 */
        channel_ref_set_cb(channel, client_cb);
    }
}

int main() {
    int i = 0;
    channel_ref_t* connector = 0;
    /* 创建循环 */
    loop_t* loop = loop_create();
    /* 创建监听者 */
    channel_ref_t* acceptor = loop_create_channel(loop, 8, 1024);
    /* 设置回调 */
    channel_ref_set_cb(acceptor, acceptor_cb);
    /* 监听 */
    channel_ref_accept(acceptor, 0, 80, 500);
    /* 连接 */
    for (; i < MAX_CONNECTOR; i++) {
        /* 创建客户端 */
        connector = loop_create_channel(loop, 8, 1024);
        /* 设置回调 */
        channel_ref_set_cb(connector, connector_cb);
        channel_ref_connect(connector, "127.0.0.1", 80, 2);
    }
    /* 启动 */
    loop_run(loop);
    /* 销毁, connector, acceptor不需要手动销毁 */
    loop_destroy(loop);
    return 0;
}

#endif /* COMPILE_MULTI_CONNECTOR */
