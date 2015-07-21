#include "example_config.h"

#if COMPILE_BASIC_C

#include "knet.h"

/**
 单个kloop_t实例, 单个连接器，单个监听者
 */

/* 客户端 - 连接器回调 */
void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    char* hello = "hello world";
    kstream_t* stream = knet_channel_ref_get_stream(channel);
    if (e & channel_cb_event_connect) { /* 连接成功 */
        /* 写入 */
        knet_stream_push(stream, hello, 12);
    }
}

/* 服务端 - 客户端回调 */
void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    char buffer[32] = {0};
    /* 获取对端地址 */
    kaddress_t* peer_addr = knet_channel_ref_get_peer_address(channel);
    kstream_t* stream = knet_channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        /* 读取 */
        knet_stream_pop(stream, buffer, sizeof(buffer));
        /* 关闭 */
        knet_channel_ref_close(channel);
        /* 退出循环 */
        knet_loop_exit(knet_channel_ref_get_loop(channel));
        printf("recv from connector: %s, ip: %s, port: %d\n", buffer,
            address_get_ip(peer_addr), address_get_port(peer_addr));
    }
}

/* 监听者回调 */
void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    if (e & channel_cb_event_accept) { /* 新连接 */
        /* 设置回调 */
        knet_channel_ref_set_cb(channel, client_cb);
    }
}

int main() {
    /* 创建循环 */
    kloop_t* loop = knet_loop_create();
    /* 创建客户端 */
    kchannel_ref_t* connector = knet_loop_create_channel(loop, 8, 1024);
    /* 创建监听者 */
    kchannel_ref_t* acceptor = knet_loop_create_channel(loop, 8, 1024);
    /* 设置回调 */
    knet_channel_ref_set_cb(connector, connector_cb);
    knet_channel_ref_set_cb(acceptor, acceptor_cb);
    /* 监听 */
    knet_channel_ref_accept(acceptor, 0, 80, 10);
    /* 连接 */
    knet_channel_ref_connect(connector, "127.0.0.1", 80, 5);
    /* 启动 */
    knet_loop_run(loop);
    /* 销毁, connector, acceptor不需要手动销毁 */
    knet_loop_destroy(loop);
    return 0;
}

#endif /* COMPILE_BASIC_C */
