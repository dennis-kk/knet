#include "example_config.h"

#if COMPILE_TELNET_ECHO

#include "knet.h"

/**
 telnet回显
 */

kloop_t* loop = 0;

/* 服务端 - 客户端回调 */
void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    int bytes = 0;
    char buffer[16] = {0};
    kstream_t* stream = knet_channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        bytes = knet_stream_available(stream);
        memset(buffer, 0, sizeof(buffer));
        /* 读取 */
        knet_stream_pop(stream, buffer, sizeof(buffer));
        if (*buffer == 'q') {
            printf("bye...\n");
            knet_loop_exit(loop);
            return;
        }
        printf("recv: %s\n", buffer);
        /* echo */
        knet_stream_push(stream, buffer, bytes);
    }
}

/* 监听者回调 */
void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    if (e & channel_cb_event_accept) { /* 新连接 */
        printf("telnet client accepted...\n");
        /* 设置回调 */
        knet_channel_ref_set_cb(channel, client_cb);
    }
}

int main() {
    kchannel_ref_t* acceptor = 0;
    loop = knet_loop_create();
    acceptor = knet_loop_create_channel(loop, 8, 1024);
    knet_channel_ref_set_cb(acceptor, acceptor_cb);
    knet_channel_ref_accept(acceptor, 0, 23, 10);
    knet_loop_run(loop);
    knet_loop_destroy(loop);
    return 0;
}

#endif /* COMPILE_TELNET_ECHO */
