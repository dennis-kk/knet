#include "example_config.h"

#if COMPILE_BROADCAST

#include "knet.h"

#define MAX_CONNECTOR 10
kbroadcast_t* broadcast = 0;
int current_count = 0;
int connector_count = MAX_CONNECTOR;

/* 服务端 - 客户端回调 */
void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    if (e & channel_cb_event_close) {
        connector_count--;
        if (connector_count == 0) {
            printf("all client closed\n");
            knet_loop_exit(knet_channel_ref_get_loop(channel));
        }
    }
}

void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {    
    char buffer[16] = {0};
    kstream_t* stream = knet_channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        memset(buffer, 0, sizeof(buffer));
        /* 读取 */
        knet_stream_pop(stream, buffer, sizeof(buffer));
        printf("recv: %s\n", buffer);
        knet_channel_ref_close(channel);
    }
}

/* 监听者回调 */
void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    if (e & channel_cb_event_accept) { /* 新连接 */
        /* 设置回调 */
        knet_channel_ref_set_cb(channel, client_cb);
        current_count++;
        /* 加入到广播组 */
        knet_broadcast_join(broadcast, channel);
        if (current_count == MAX_CONNECTOR) {
            /* 全部连接完成，广播 */
            knet_broadcast_write(broadcast, "hello world", 12);
        }
    }
}

int main() {
    int i = 0;
    kloop_t* loop = knet_loop_create();
    kchannel_ref_t* acceptor = 0;
    kchannel_ref_t* connector = 0;
    acceptor = knet_loop_create_channel(loop, 8, 1024);
    knet_channel_ref_accept(acceptor, 0, 8000, 50);
    knet_channel_ref_set_cb(acceptor, acceptor_cb);
    broadcast = knet_broadcast_create();
    for (; i < MAX_CONNECTOR; i++) {
        connector = knet_loop_create_channel(loop, 8, 1024);
        knet_channel_ref_set_cb(connector, connector_cb);
        knet_channel_ref_connect(connector, "127.0.0.1", 8000, 0);
    }

    knet_loop_run(loop);
    knet_broadcast_destroy(broadcast);
    knet_loop_destroy(loop);

    return 0;
}

#endif /* COMPILE_BROADCAST */
