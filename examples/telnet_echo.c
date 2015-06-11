#include "example_config.h"

#if COMPILE_TELNET_ECHO

#include "knet.h"

/*
 telnet回显
 */

loop_t* loop = 0;

/* 服务端 - 客户端回调 */
void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    int bytes = 0;
    char buffer[16] = {0};
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        memset(buffer, 0, sizeof(buffer));
        /* 读取 */
        bytes = stream_pop(stream, buffer, sizeof(buffer));
        if (*buffer == 'q') {
            printf("bye...\n");
            loop_exit(loop);
            return;
        }
        printf("recv: %s\n", buffer);
        /* echo */
        stream_push(stream, buffer, bytes);
    }
}

/* 监听者回调 */
void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    if (e & channel_cb_event_accept) { /* 新连接 */
        printf("telnet client accepted...\n");
        /* 设置回调 */
        channel_ref_set_cb(channel, client_cb);
    }
}

int main() {
    channel_ref_t* acceptor = 0;
    loop = loop_create();
    acceptor = loop_create_channel(loop, 8, 1024);
    channel_ref_set_cb(acceptor, acceptor_cb);
    channel_ref_accept(acceptor, 0, 23, 10);
    loop_run(loop);
    loop_destroy(loop);
    return 0;
}

#endif /* COMPILE_TELNET_ECHO */
