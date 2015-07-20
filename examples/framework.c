#include "example_config.h"

#if COMPILE_FRAMEWORK_C

#include "knet.h"

framework_t* f = 0;

/* 服务端 - 客户端回调 */
void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    int bytes = 0;
    char buffer[16] = {0};
    stream_t* stream = channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) { /* 有数据可以读 */
        bytes = stream_available(stream);
        memset(buffer, 0, sizeof(buffer));
        /* 读取 */
        stream_pop(stream, buffer, sizeof(buffer));
        if (*buffer == 'q') {
            printf("bye...\n");
            framework_stop(f);
            return;
        }
        printf("recv: %s\n", buffer);
        /* echo */
        stream_push(stream, buffer, bytes);
    }
}

int main() {
    framework_config_t* c = 0;
    framework_acceptor_config_t* ac = 0;
    f = framework_create();
    c = framework_get_config(f);
    ac = framework_config_new_acceptor(c);
    framework_acceptor_config_set_local_address(ac, 0, 23);
    framework_acceptor_config_set_client_cb(ac, client_cb);
    /* 启动框架, 等待关闭，销毁*/
    framework_start_wait_destroy(f);
    return 0;
}

#endif /* COMPILE_FRAMEWORK_C */