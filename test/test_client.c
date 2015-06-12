#include "knet.h"

uint32_t active_channel = 0;
uint32_t recv_bytes = 0;
uint32_t send_bytes = 0;
uint32_t client_n   = 50;
char*    ip         = 0;
int      port       = 0;
ktimer_loop_t* timer_loop = 0;

void timer_cb(ktimer_t* timer, void* data) {
    printf("Active channel: %d, Recv: %d, Send: %d\n", active_channel, recv_bytes, send_bytes);
}

void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    channel_ref_t* connector  = 0;
    char      buffer[1024]    = {0};
    char*     hello           = "hello world";
    int       bytes           = 0;
    stream_t* stream          = channel_ref_get_stream(channel);
    if (e & channel_cb_event_connect) { /* 连接成功 */
        active_channel++;
        /* 写入 */
        send_bytes += 12;
        stream_push(stream, hello, 12);
        if (active_channel < client_n) {
            connector = loop_create_channel(channel_ref_get_loop(channel), 8, 121);
            channel_ref_set_cb(connector, connector_cb);
            channel_ref_set_timeout(connector, 1);
            channel_ref_connect(connector, ip, port, 120);
        }
    } else if (e & channel_cb_event_recv) {
        bytes = stream_available(stream);
        if (error_ok == stream_pop(stream, buffer, bytes)) {
            recv_bytes += bytes;
        }
    } else if (e & channel_cb_event_close) {
        active_channel--;
        printf("unexpect close\n");
    } else if (e & channel_cb_event_connect_timeout) {
        printf("connect timeout!\n");
        channel_ref_close(channel);
    } else if (e & channel_cb_event_timeout) {
        /* 写入 */
        if (error_ok == stream_push(stream, hello, 12)) {
            send_bytes += 12;
        }
    }
}

int main(int argc, char* argv[]) {
    int              i            = 0;
    loop_t*          loop         = 0;
    ktimer_t*        timer        = 0;
    channel_ref_t*   connector    = 0;
    thread_runner_t* timer_thread = 0;

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
    }

    loop       = loop_create();
    timer_loop = ktimer_loop_create(1000, 1000);
    timer      = ktimer_create(timer_loop);

    ktimer_start(timer, timer_cb, 0, 1000);
    timer_thread = thread_runner_create(0, 0);
    thread_runner_start_timer_loop(timer_thread, timer_loop, 0);

    connector = loop_create_channel(loop, 8, 121);
    channel_ref_set_cb(connector, connector_cb);
    channel_ref_set_timeout(connector, 1);
    if (error_ok != channel_ref_connect(connector, ip, port, 120)) {
        return 0;
    }

    loop_run(loop);
    thread_runner_destroy(timer_thread);
    loop_destroy(loop);
    ktimer_loop_destroy(timer_loop);

    return 0;
}
