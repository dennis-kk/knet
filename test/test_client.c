#include "knet.h"

#if defined(_MSC_VER )
#pragma comment(lib,"Ws2_32.lib")
#endif /* defined(_MSC_VER) */

uint32_t active_channel = 0;
uint32_t recv_bytes = 0;
uint32_t send_bytes = 0;
uint32_t client_n   = 50;
char*    ip         = 0;
int      port       = 0;
ktimer_loop_t* timer_loop = 0;

void client_timer_cb(ktimer_t* timer, void* data) {
	(void)data;
    assert(timer);
    printf("Active channel: %d, Recv: %d, Send: %d\n", active_channel, recv_bytes, send_bytes);
}

void connector_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    char      buffer[1024]    = {0};
    char*     hello           = "hello world";
    int       bytes           = 0;
    kstream_t* stream          = knet_channel_ref_get_stream(channel);
    if (e & channel_cb_event_connect) { /* 连接成功 */
        printf("connected: %d\n", knet_channel_ref_get_socket_fd(channel));
        active_channel++;
        /* 写入 */
        send_bytes += 12;
        knet_stream_push(stream, hello, 12);
    } else if (e & channel_cb_event_recv) {
        bytes = knet_stream_available(stream);
        if (error_ok == knet_stream_pop(stream, buffer, bytes)) {
            recv_bytes += bytes;
        }
    } else if (e & channel_cb_event_close) {
        if (active_channel > 0) {
            active_channel--;
        }
        printf("unexpect close\n");
    } else if (e & channel_cb_event_connect_timeout) {
        printf("connect timeout!\n");
        knet_channel_ref_close(channel);
    } else if (e & channel_cb_event_timeout) {
        /* 写入 */
        if (error_ok == knet_stream_push(stream, hello, 12)) {
            send_bytes += 12;
        }
    }
}

int main(int argc, char* argv[]) {
    int                i            = 0;
    kloop_t*            loop         = 0;
    ktimer_t*          timer        = 0;
    kchannel_ref_t*     connector    = 0;
    kthread_runner_t*   timer_thread = 0;
    static const char* helper_string =
        "-n    client count\n"
        "-ip   remote host IP\n"
        "-port remote host port\n";

    if (argc > 2) {
        for (i = 1; i < argc; i++) {
            if (!strcmp("-n", argv[i])) {
                client_n = atoi(argv[i+1]);
            } else if (!strcmp("-ip", argv[i])) {
                ip = argv[i+1];
            } else if (!strcmp("-port", argv[i])) {
                port = atoi(argv[i+1]);
            }
        }
    } else {
        printf(helper_string);
        exit(0);
    }

    loop       = knet_loop_create();
    timer_loop = ktimer_loop_create(1000);
    timer      = ktimer_create(timer_loop);

    ktimer_start(timer, client_timer_cb, 0, 1000);
    timer_thread = thread_runner_create(0, 0);
    thread_runner_start_timer_loop(timer_thread, timer_loop, 0);

    for (i = 0; i < client_n; i++) {
        connector = knet_loop_create_channel(loop, 64, 1024);
        knet_channel_ref_set_cb(connector, connector_cb);
        knet_channel_ref_set_timeout(connector, 1);
        if (error_ok != knet_channel_ref_connect(connector, ip, port, 10)) {
            knet_channel_ref_close(connector);
            return 0;
        }
    }

    knet_loop_run(loop);
    thread_runner_destroy(timer_thread);
    knet_loop_destroy(loop);
    ktimer_loop_destroy(timer_loop);

    return 0;
}
