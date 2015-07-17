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
 * DISCLAIMED. IN NO EVENT SHALL dennis wang BE LIABLE FOR ANY
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
#include "loop_balancer.h"
#include "misc.h"

struct _framework_worker_t {
    loop_t*          loop;   /* 网络事件循环 */
    framework_t*     f;      /* 框架 */
    thread_runner_t* runner; /* 线程 */
};

framework_worker_t* framework_worker_create(framework_t* f, loop_t* loop) {
    framework_worker_t* worker = 0;
    verify(f);
    verify(loop);
    worker = create(framework_worker_t);
    verify(worker);
    memset(worker, 0, sizeof(framework_worker_t));
    worker->f    = f;
    worker->loop = loop;
    return worker;    
}

void framework_worker_destroy(framework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        if (thread_runner_check_start(worker->runner)) {
            thread_runner_stop(worker->runner);
        }
        thread_runner_destroy(worker->runner);
    }
    destroy(worker);
}

int framework_worker_start(framework_worker_t* worker) {
    verify(worker);
    worker->runner = thread_runner_create(0, 0);
    verify(worker->runner);
    return thread_runner_start_loop(worker->runner, worker->loop, 0);
}

void framework_worker_stop(framework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        if (thread_runner_check_start(worker->runner)) {
            thread_runner_stop(worker->runner);
        }
    }
}

void framework_worker_wait_for_stop(framework_worker_t* worker) {
    verify(worker);
    if (worker->runner) {
        thread_runner_join(worker->runner);
    }
}
