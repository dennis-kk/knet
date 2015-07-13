#include "framework_acceptor.h"
#include "loop.h"
#include "loop_balancer.h"
#include "framework.h"
#include "channel_ref.h"
#include "misc.h"

struct _framework_acceptor_t {
    loop_t*          loop;   /* 网络事件循环 */
    framework_t*     f;      /* 框架 */
    thread_runner_t* runner; /* 线程 */
};

framework_acceptor_t* framework_acceptor_create(framework_t* f) {    
    framework_acceptor_t* a = 0;
    verify(f);
    a = create(framework_acceptor_t);
    verify(a);
    memset(a, 0, sizeof(framework_acceptor_t));
    a->f    = f;
    a->loop = loop_create();
    verify(a->loop);
    /* 所有接受的新管道全部给其他loop_t处理 */
    loop_set_balance_options(a->loop, loop_balancer_out);
    return a;
}

void framework_acceptor_destroy(framework_acceptor_t* a) {
    verify(a);
    if (a->loop) {
        loop_destroy(a->loop);
    }
    destroy(a);
}

int framework_acceptor_start(framework_acceptor_t* a) {
    int                 error   = 0;
    channel_ref_t*      channel = 0;
    framework_config_t* config  = 0;
    verify(a);
    config  = framework_get_config(a->f);
    verify(config);
    /* 建立监听管道 */
    channel = loop_create_channel(a->loop, framework_config_get_max_send_list(config),
        framework_config_get_max_recv_buffer(config));
    verify(channel);
    channel_ref_set_cb(channel, acceptor_cb);
    /* 监听 */
    error = channel_ref_accept(channel, framework_config_get_ip(config),
        framework_config_get_port(config), framework_config_get_backlog(config));
    if (error != error_ok) {
        channel_ref_destroy(channel);
        return error;
    }
    a->runner = thread_runner_create(0, 0);
    verify(a->runner);
    /* 关联到负载均衡器，监听器所有新连接都会被分派到工作线程处理 */
    loop_balancer_attach(framework_get_balancer(a->f), a->loop);
    /* 启动网络事件循环 */
    return thread_runner_start_loop(a->runner, a->loop, 0);
}

void framework_acceptor_stop(framework_acceptor_t* a) {
    verify(a);
    if (a->runner) {
        if (thread_runner_check_start(a->runner)) {
            thread_runner_stop(a->runner);
        }
    }
}

void framework_acceptor_wait_for_stop(framework_acceptor_t* a) {
    verify(a);
    if (a->runner) {
        thread_runner_join(a->runner);
        thread_runner_destroy(a->runner);
    }
}

void acceptor_cb(channel_ref_t* channel, channel_cb_event_e e) {
    loop_t*          loop     = channel_ref_get_loop(channel);
    loop_balancer_t* balancer = loop_get_balancer(loop);
    framework_t*     f        = (framework_t*)loop_balancer_get_data(balancer);
    if (e & channel_cb_event_accept) {
        /* 设置用户回调 */
        channel_ref_set_cb(channel, framework_get_cb(f));
    }
}
