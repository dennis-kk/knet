/*
 * Copyright (c) 2014-2015, dennis wang
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "framework_worker.h"
#include "framework.h"
#include "loop.h"
#include "timer.h"
#include "loop_balancer.h"
#include "misc.h"

struct _framework_worker_t {
    kloop_t*          loop;       /* 网络事件循环 */
    ktimer_loop_t*   timer_loop; /* 定时器循环 */
    kframework_t*     f;          /* 框架 */
    kthread_runner_t* runner;     /* 线程 */
};

kframework_worker_t* knet_framework_worker_create(kframework_t* f, kloop_t* loop) {
    kframework_worker_t* worker = 0;
    verify(f);
    verify(loop);
    worker = create(kframework_worker_t);
    verify(worker);
    memset(worker, 0, sizeof(kframework_worker_t));
    worker->f    = f;
    worker->loop = loop;
    /* 建立一个内置的定时器循环 */
    worker->timer_loop = ktimer_loop_create(
        framework_config_get_worker_timer_freq(knet_framework_get_config(f)),
        framework_config_get_worker_timer_slot(knet_framework_get_config(f)));
    verify(worker->timer_loop);
    return worker;
}

void knet_framework_worker_destroy(kframework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        if (thread_runner_check_start(worker->runner)) {
            thread_runner_stop(worker->runner);
        }
        thread_runner_destroy(worker->runner);
    }
    if (worker->timer_loop) {
        ktimer_loop_destroy(worker->timer_loop);
    }
    destroy(worker);
}

int knet_framework_worker_start(kframework_worker_t* worker) {
    verify(worker);
    worker->runner = thread_runner_create(0, 0);
    verify(worker->runner);
    /* 启动一个线程，运行一个kloop_t，一个ktimer_loop_t */
    return thread_runner_start_multi_loop_varg(worker->runner, 0, "lt",
        worker->loop, worker->timer_loop);
}

void knet_framework_worker_stop(kframework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        if (thread_runner_check_start(worker->runner)) {
            thread_runner_stop(worker->runner);
        }
    }
}

void knet_framework_worker_wait_for_stop(kframework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        thread_runner_join(worker->runner);
    }
}

ktimer_t* knet_framework_worker_create_timer(kframework_worker_t* worker) {
    ktimer_t* timer = 0;
    verify(worker);
    verify(worker->timer_loop);
    timer = ktimer_create(worker->timer_loop);
    verify(timer);
    return timer;
}

thread_id_t knet_framework_worker_get_id(kframework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        return thread_runner_get_id(worker->runner);
    }
    return 0;
}
