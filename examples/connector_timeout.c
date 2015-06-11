#include "example_config.h"

#if COMPILE_CONNECTOR_TIMEOUT

#include "knet.h"

/* 客户端 - 连接器回调 */
void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    if (e & channel_cb_event_connect_timeout) { /* 连接成功 */
        channel_ref_close(channel);
        /* 连接超时，退出循环 */
        loop_exit(channel_ref_get_loop(channel));
    } else if (e & channel_cb_event_close) {
        /* 发生错误，退出循环 */
        printf("connect failed!\n");
        loop_exit(channel_ref_get_loop(channel));
    }
}

int main() {
    /* 创建循环 */
    loop_t* loop = loop_create();
    /* 创建客户端 */
    channel_ref_t* connector = loop_create_channel(loop, 8, 1024);
    /* 设置回调 */
    channel_ref_set_cb(connector, connector_cb);
    /* 连接 */
    if (error_ok != channel_ref_connect(connector, "127.0.0.1", 8000, 2)) {
        printf("remote unreachable\n");
    } else {
        /* 启动 */
        loop_run(loop);
    }
    /* 销毁, connector, acceptor不需要手动销毁 */
    loop_destroy(loop);
    return 0;
}

#endif /* COMPILE_CONNECTOR_TIMEOUT */
