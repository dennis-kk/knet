#include "framework_worker.h"
#include "framework.h"
#include "loop.h"
#include "loop_balancer.h"
#include "misc.h"

struct _framework_worker_t {
    loop_t*          loop;   /* 网络事件循环 */
    framework_t*     f;      /* 框架 */
    thread_runner_t* runner; /* 线程 */
};

framework_worker_t* framework_worker_create(framework_t* f) {
    framework_worker_t* w = 0;
    verify(f);
    w = create(framework_worker_t);
    verify(w);
    memset(w, 0, sizeof(framework_worker_t));
    w->f = f;
    return w;    
}

void framework_worker_destroy(framework_worker_t* w) {
    verify(w);    
    if (w->loop) {
        loop_destroy(w->loop);
    }
    destroy(w);
}

int framework_worker_start(framework_worker_t* w) {
    verify(w);
    w->loop = loop_create();
    verify(w->loop);
    w->runner = thread_runner_create(0, 0);
    verify(w->runner);
    /* 关联到负载均衡器，监听器所有新连接都会被分派到工作线程处理 */
    loop_balancer_attach(framework_get_balancer(w->f), w->loop);
    return thread_runner_start_loop(w->runner, w->loop, 0);
}

void framework_worker_stop(framework_worker_t* w) {
    verify(w);
    if (w->runner) {
        if (thread_runner_check_start(w->runner)) {
            thread_runner_stop(w->runner);
        }
    }
}

void framework_worker_wait_for_stop(framework_worker_t* w) {
    verify(w);
    thread_runner_join(w->runner);
    thread_runner_destroy(w->runner);
}
