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

#ifdef LOOP_EPOLL

#include "loop.h"
#include "list.h"
#include "channel_ref.h"
#include "channel.h"

typedef struct _loop_epoll_t {
    int                 epoll_fd; /* epoll描述符 */
    struct epoll_event* events;   /* epoll事件数组 */
} loop_epoll_t;

#define MAXEVENTS 8192

int impl_create(loop_t* loop) {
    loop_epoll_t* impl = create(loop_epoll_t);
    loop_set_impl(loop, impl);
    impl->epoll_fd = epoll_create(MAXEVENTS);
    if (impl->epoll_fd < 0) {
        destroy(impl);
        return 1;
    }
    impl->events = create_type(struct epoll_event, sizeof(struct epoll_event) * MAXEVENTS);
    assert(impl->events);
    return error_ok;
}

void impl_destroy(loop_t* loop) {
    loop_epoll_t* impl = (loop_epoll_t*)loop_get_impl(loop);
    close(impl->epoll_fd);
    destroy(impl->events);
    destroy(impl);
}

int _select(loop_t* loop, int* count) {
    loop_epoll_t* impl = (loop_epoll_t*)loop_get_impl(loop);
    *count = epoll_wait(impl->epoll_fd, impl->events, MAXEVENTS, 1);
    if (*count < 0) {
        return error_loop_fail;
    }
    return error_ok;
}

int impl_run_once(loop_t* loop) {
    int count = 0;
    int i = 0;
    channel_ref_t* channel_ref = 0;
    struct epoll_event event;
    time_t ts = time(0);
    struct epoll_event* events = 0;
    loop_epoll_t* impl = (loop_epoll_t*)loop_get_impl(loop);
    int error = _select(loop, &count);
    if (error != error_ok) {
        return error;
    }
    events = impl->events;
    for (; i < count; i++) {
        channel_ref = (channel_ref_t*)events[i].data.ptr;
        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
           /* ManPage: In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required a non-NULL pointer
              in event, even though this argument is ignored. Since Linux 2.6.9, event can be specified
              as NULL when using EPOLL_CTL_DEL. Applications that need to be portable to kernels before
              2.6.9 should specify a non-NULL pointer in event.
            */
            epoll_ctl(impl->epoll_fd, EPOLL_CTL_DEL, channel_ref_get_socket_fd(channel_ref), &event);
        } else if (events[i].events & EPOLLIN) {
            channel_ref_update(channel_ref, channel_event_recv, ts);
        } else if (events[i].events & EPOLLOUT) {
            channel_ref_update(channel_ref, channel_event_send, ts);
        } else {
        }
    }
    loop_check_timeout(loop, ts);
    loop_check_close(loop);
    return error_ok;
}

int impl_event_add(channel_ref_t* channel_ref, channel_event_e e) {
    struct epoll_event event;
    loop_epoll_t* impl = (loop_epoll_t*)loop_get_impl(channel_ref_get_loop(channel_ref));
    channel_event_e old_event = channel_ref_get_event(channel_ref);
    memset(&event, 0, sizeof(event));
    event.data.ptr = channel_ref;
    event.events |= EPOLLET;
    if (e & channel_event_recv) {
        if (old_event & channel_event_send) { /* 已经注册写事件 */
            event.events |= EPOLLOUT;
        }
        event.events |= EPOLLIN;
    }
    if (e & channel_event_send) {
        if (old_event & channel_event_recv) { /* 已经注册读事件 */
            event.events |= EPOLLIN;
        }
        event.events |= EPOLLOUT;
    }
    if (channel_ref_get_flag(channel_ref)) {
        epoll_ctl(impl->epoll_fd, EPOLL_CTL_MOD, channel_ref_get_socket_fd(channel_ref), &event);
    } else {
        channel_ref_set_flag(channel_ref, 1);
        epoll_ctl(impl->epoll_fd, EPOLL_CTL_ADD, channel_ref_get_socket_fd(channel_ref), &event);
    }
    return error_ok;
}

int impl_event_remove(channel_ref_t* channel_ref, channel_event_e e) {
    struct epoll_event event;
    loop_epoll_t* impl = (loop_epoll_t*)loop_get_impl(channel_ref_get_loop(channel_ref));
    channel_event_e old_event = channel_ref_get_event(channel_ref);
    memset(&event, 0, sizeof(event));
    event.data.ptr = channel_ref;
    event.events |= EPOLLET;
    if (e & channel_event_recv) {
        if (old_event & channel_event_send) { /* 已经注册写事件 */
            event.events |= EPOLLOUT;
        }
    } 
    if (e & channel_event_send) {
        if (old_event & channel_event_recv) { /* 已经注册读事件 */
            event.events |= EPOLLIN;
        }
    }
    if (channel_ref_get_flag(channel_ref)) {
        epoll_ctl(impl->epoll_fd, EPOLL_CTL_MOD, channel_ref_get_socket_fd(channel_ref), &event);
    } else {
        channel_ref_set_flag(channel_ref, 1);
        epoll_ctl(impl->epoll_fd, EPOLL_CTL_ADD, channel_ref_get_socket_fd(channel_ref), &event);
    }
    return error_ok;
}

int impl_add_channel_ref(loop_t* loop, channel_ref_t* channel_ref) {
    return error_ok;
}

int impl_remove_channel_ref(loop_t* loop, channel_ref_t* channel_ref) {
    /* 清除添加标记 */
    channel_ref_set_flag(channel_ref, 0);
    return error_ok;
}

socket_t impl_channel_accept(channel_ref_t* channel_ref) {
    return 0;
}

#endif
