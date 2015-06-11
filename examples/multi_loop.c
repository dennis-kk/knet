#include "example_config.h"

#if COMPILE_MULTI_LOOP

#include "knet.h"
#include "misc.h"

/*
 4线程+主线程
 */

#define MAX_CLIENT 200
#define MAX_ECHO_COUNT MAX_CLIENT * 100
#define MAX_LOOP 4
#define TEST_TIMES 100
int recv_count = 0;
atomic_counter_t client_count = 0;

void client_cb(channel_ref_t* channel, channel_cb_event_e e) {
    address_t* peer_address = 0;
    char buffer[100] = {0};
    stream_t* stream = 0;
    int bytes = 0;
    if (e & channel_cb_event_recv) {
        stream = channel_ref_get_stream(channel);
        bytes = stream_pop(stream, buffer, sizeof(buffer));
        stream_push(stream, buffer, bytes);
        recv_count++;
        if (recv_count > MAX_ECHO_COUNT) {
            peer_address = channel_ref_get_peer_address(channel);
            printf("client reach max echo, close: %s, %d\n", address_get_ip(peer_address), address_get_port(peer_address));
            channel_ref_close(channel);
        }
    } else if (e & channel_cb_event_timeout) {
        peer_address = channel_ref_get_peer_address(channel);
        printf("client timeout, close: %s, %d\n", address_get_ip(peer_address), address_get_port(peer_address));
        channel_ref_close(channel);
    }
}

int total_connected = 0;

void connector_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[10] = {0};
    stream_t* stream = 0;
    int bytes = 0;
    if (e & channel_cb_event_connect) {
        total_connected++;
        printf("connect finished: %d, %d\n", channel_ref_get_socket_fd(channel), total_connected);
    } else if (e & channel_cb_event_recv) {
        stream = channel_ref_get_stream(channel);
        bytes = stream_pop(stream, buffer, sizeof(buffer));
        stream_push(stream, buffer, bytes);
    } else if (e & channel_cb_event_close) {
        atomic_counter_dec(&client_count);
        if (client_count == 0) {
            printf("all client closed\n");
            loop_exit(channel_ref_get_loop(channel));
        }
    } else if (e & channel_cb_event_connect_timeout) {
        printf("connector close, connect timeout: %d\n", channel_ref_get_socket_fd(channel));
        channel_ref_close(channel);
    } else if (e & channel_cb_event_timeout) {
        printf("connector recv timeout, close: %d\n", channel_ref_get_socket_fd(channel));
        channel_ref_close(channel);
    }
}

void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    char buffer[100] = {0};
    stream_t* stream = 0;
    if (e & channel_cb_event_accept) {
        channel_ref_set_cb(channel, client_cb);
        channel_ref_set_timeout(channel, 2);
        stream = channel_ref_get_stream(channel);
        stream_push(stream, buffer, sizeof(buffer));
    }
}

int main() {
    int i = 0;
    int error = 0;
    loop_t* main_loop = 0;
    loop_t* sub_loop[MAX_LOOP] = {0};
    thread_runner_t* runner[MAX_LOOP] = {0};
    channel_ref_t* acceptor = 0;
    channel_ref_t* connector = 0;
    loop_balancer_t* balancer = 0;
    int times = 0;

    /* 建立一个负载均衡器 */
    balancer = loop_balancer_create();
    /* 创建多个线程，每个线程运行一个loop_t */
    for (i = 0; i < MAX_LOOP; i++) {
        sub_loop[i] = loop_create();
        loop_balancer_attach(balancer, sub_loop[i]);
        runner[i] = thread_runner_create(0, 0);
        thread_runner_start_loop(runner[i], sub_loop[i], 0);
    }
    main_loop = loop_create();
    loop_balancer_attach(balancer, main_loop);

    acceptor = loop_create_channel(main_loop, 8, 1024 * 8);
    channel_ref_set_cb(acceptor, acceptor_cb);
    error = channel_ref_accept(acceptor, 0, 80, 5000);
    if (error_ok != error) {
        printf("channel_ref_accept failed: %d\n", error);
    }
    
    /* 多次测试 */
    for (; times < TEST_TIMES; times++) {
        total_connected = 0;
        recv_count = 0;
        client_count = MAX_CLIENT;
        for (i = 0; i < MAX_CLIENT; i++) {
            connector = loop_create_channel(main_loop, 8, 8192);
            channel_ref_set_cb(connector, connector_cb);
            channel_ref_set_timeout(connector, 2);
            channel_ref_connect(connector, "127.0.0.1", 80, 2);
        }
        while (client_count > 0) {
            /* 主线程 */
            error = loop_run_once(main_loop);
        }
        if (error != error_ok) {
            printf("loop_run() failed: %d\n", error);
        }
    }
    for (i = 0; i < MAX_LOOP; i++) {
        thread_runner_stop(runner[i]);
        thread_runner_join(runner[i]);
        thread_runner_destroy(runner[i]);
    }
    for (i = 0; i < MAX_LOOP; i++) {
        loop_destroy(sub_loop[i]);
    }
    loop_destroy(main_loop);
    loop_balancer_destroy(balancer);
    return 0;
}

#endif /* COMPILE_MULTI_LOOP */
