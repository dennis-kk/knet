#include "example_config.h"

#if COMPILE_BASIC_C

#include "knet.h"

/* 客户端 - 连接器回调 */
void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char* hello = "hello world";
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_connect) { /* 连接成功 */
        /* 写入 */
        stream_push(stream, hello, 12);
    }
}

/* 服务端 - 客户端回调 */
void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[32] = {0};
    /* 获取对端地址 */
    address_t* peer_addr = channel_ref_get_peer_address(channel);
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        /* 读取 */
        stream_pop(stream, buffer, sizeof(buffer));
        /* 关闭 */
        channel_ref_close(channel);
        /* 退出循环 */
        loop_exit(channel_ref_get_loop(channel));
        printf("recv from connector: %s, ip: %s, port: %d\n", buffer,
            address_get_ip(peer_addr), address_get_port(peer_addr));
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
    /* 创建循环 */
    loop_t* loop = loop_create();
    /* 创建客户端 */
    channel_ref_t* connector = loop_create_channel(loop, 8, 1024);
    /* 创建监听者 */
    channel_ref_t* acceptor = loop_create_channel(loop, 8, 1024);
    /* 设置回调 */
    channel_ref_set_cb(connector, connector_cb);
    channel_ref_set_cb(acceptor, acceptor_cb);
    /* 监听 */
    channel_ref_accept(acceptor, 0, 80, 10);
    /* 连接 */
    channel_ref_connect(connector, "127.0.0.1", 80, 5);
    /* 启动 */
    loop_run(loop);
    /* 销毁, connector, acceptor不需要手动销毁 */
    loop_destroy(loop);
    return 0;
}

#endif /* COMPILE_BASIC_C */
