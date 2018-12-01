#include "knet.h"

#if defined(_MSC_VER )
#pragma comment(lib,"Ws2_32.lib")
#endif /* defined(_MSC_VER) */

void client_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    char      buffer[1024] = {0};
    int       bytes      = 0;
    kstream_t* stream     = knet_channel_ref_get_stream(channel);
    if (e & channel_cb_event_recv) {
        bytes = knet_stream_available(stream);
        if (error_ok == knet_stream_pop(stream, buffer, sizeof(buffer))) {
            /* echoÐ´Èë */
            if (bytes) {
                knet_stream_push(stream, buffer, bytes);
            }
        }        
    } else if (e & channel_cb_event_close) {
        printf("active channel count: %d, close channel count: %d\n",
            knet_loop_get_active_channel_count(knet_channel_ref_get_loop(channel)),
            knet_loop_get_close_channel_count(knet_channel_ref_get_loop(channel)));
    } else if (e & channel_cb_event_timeout) {
        printf("client channel timeout\n");
        knet_channel_ref_close(channel);
    }
}

void acceptor_cb(kchannel_ref_t* channel, knet_channel_cb_event_e e) {
    kaddress_t*  address = 0;
    if (e & channel_cb_event_accept) {
        printf("accept fd: %d, active channel count: %d, close channel count: %d\n",
            knet_channel_ref_get_socket_fd(channel),
            knet_loop_get_active_channel_count(knet_channel_ref_get_loop(channel)),
            knet_loop_get_close_channel_count(knet_channel_ref_get_loop(channel)));
        knet_channel_ref_set_timeout(channel, 120);
        knet_channel_ref_set_cb(channel, client_cb);
        /*address = knet_channel_ref_get_peer_address(channel);
        printf("peer: ip:%s, port:%d\n", address_get_ip(address), address_get_port(address));
        address = knet_channel_ref_get_local_address(channel);
        printf("local: ip:%s, port:%d\n", address_get_ip(address), address_get_port(address));*/
    }
}

int isIPV6(const char* ip) {
    if (!strchr(ip, ':')) {
        return 0;
    }
    return 1;
}

int main(int argc, char* argv[]) {
    int               i        = 0;
    int               worker   = 0;
    char*             ip       = 0;
    int               port     = 0;
    kloop_t*           loop     = 0;
    kloop_t**          loops    = 0;
    kloop_balancer_t*  balancer = 0;
    kchannel_ref_t*    acceptor = 0;
    kthread_runner_t** threads  = 0;

    static const char* helper_string =
        "-w    loop worker count\n"
        "-ip   host IP\n"
        "-port listening port\n";

    if (argc > 2) {
        for (i = 1; i < argc; i++) {
            if (!strcmp("-ip", argv[i])) {
                ip = argv[i+1];
            } else if (!strcmp("-port", argv[i])) {
                port = atoi(argv[i+1]);
            } else if (!strcmp("-w", argv[i])) {
                worker = atoi(argv[i+1]);
            }
        }
    } else {
        printf(helper_string);
        exit(0);
    }

    balancer = knet_loop_balancer_create();
    loop     = knet_loop_create();
    threads  = (kthread_runner_t**)malloc(sizeof(kthread_runner_t*) * worker);
    loops    = (kloop_t**)malloc(sizeof(kloop_t*) * worker);

    knet_loop_balancer_attach(balancer, loop);

    if (isIPV6(ip)) {
        acceptor = knet_loop_create_channel6(loop, 8, 1024);
    } else {
        acceptor = knet_loop_create_channel(loop, 8, 1024);
    }
    knet_channel_ref_set_cb(acceptor, acceptor_cb);
    knet_channel_ref_accept(acceptor, ip, port, 5000);
    for (i = 0; i < worker; i++) {
        loops[i] = knet_loop_create();
        knet_loop_balancer_attach(balancer, loops[i]);
        threads[i] = thread_runner_create(0, 0);
        assert(threads[i]);
        thread_runner_start_loop(threads[i], loops[i], 0);
    }

    knet_loop_run(loop);

    for (i = 0; i < worker; i++) {
        thread_runner_destroy(threads[i]);        
    }
    for (i = 0; i < worker; i++) {
        knet_loop_destroy(loops[i]);
    }
    knet_loop_destroy(loop);
    knet_loop_balancer_destroy(balancer);
    if (loops) {
        free(loops);
    }
    if (threads) {
        free(threads);
    }
    return 0;
}
