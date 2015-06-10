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

#ifndef MISC_H
#define MISC_H

#include "config.h"

socket_t socket_create();
int socket_connect(socket_t socket_fd, const char* ip, int port);
int socket_bind_and_listen(socket_t socket_fd, const char* ip, int port, int backlog);
socket_t socket_accept(socket_t socket_fd);
int socket_close(socket_t socket_fd);
int socket_set_reuse_addr_on(socket_t socket_fd);
int socket_set_non_blocking_on(socket_t socket_fd);
int socket_set_nagle_off(socket_t socket_fd);
int socket_set_linger_off(socket_t socket_fd);
int socket_set_keepalive_off(socket_t socket_fd);
int socket_set_donot_route_on(socket_t socket_fd);
int socket_set_recv_buffer_size(socket_t socket_fd, int size);
int socket_set_send_buffer_size(socket_t socket_fd, int size);
int socket_send(socket_t socket_fd, const char* data, uint32_t size);
int socket_recv(socket_t socket_fd, char* data, uint32_t size);
int socket_pair(socket_t pair[2]);
int socket_getpeername(channel_ref_t* channel_ref, address_t* address);
int socket_getsockname(channel_ref_t* channel_ref, address_t* address);
int socket_check_send_ready(socket_t socket_fd);

atomic_counter_t atomic_counter_inc(atomic_counter_t* counter);
atomic_counter_t atomic_counter_dec(atomic_counter_t* counter);
int atomic_counter_zero(atomic_counter_t* counter);

lock_t* lock_create();
void lock_destroy(lock_t* lock);
void lock_lock(lock_t* lock);
int lock_trylock(lock_t* lock);
void lock_unlock(lock_t* lock);

struct _thread_runner_t {
    thread_func_t func;
    void* params;
    volatile int running;
    thread_id_t thread_id;
};

thread_runner_t* thread_runner_create(thread_func_t func, void* params);
void thread_runner_destroy(thread_runner_t* runner);
int thread_runner_start(thread_runner_t* runner, int stack_size);
void thread_runner_stop(thread_runner_t* runner);
int thread_runner_start_loop(thread_runner_t* runner, loop_t* loop, int stack_size);
void thread_runner_join(thread_runner_t* runner);
int thread_runner_check_start(thread_runner_t* runner);
void* thread_runner_get_params(thread_runner_t* runner);
thread_id_t thread_get_self_id();
void thread_sleep_ms(int ms);

uint32_t time_get_milliseconds();
uint64_t time_get_microseconds();
uint64_t gen_domain_uuid();

#endif /* MISC_H */
