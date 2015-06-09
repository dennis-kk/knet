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

#ifdef LOOP_SELECT

#include "loop.h"
#include "list.h"
#include "channel_ref.h"

typedef struct _loop_select_t {
    fd_set read_fds[FD_SETSIZE]; /* select读描述符数组 */
    fd_set send_fds[FD_SETSIZE]; /* select写描述符数组 */
} loop_select_t;

int impl_create(loop_t* loop) {
#if defined(WIN32) || defined(WIN64)
    WSADATA wsa;
#endif /* defined(WIN32) || defined(WIN64) */
    loop_select_t* impl = create(loop_select_t);
    loop_set_impl(loop, impl);
#if defined(WIN32) || defined(WIN64)
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif /* defined(WIN32) || defined(WIN64) */
    return error_ok;
}

void impl_destroy(loop_t* loop) {
    destroy(loop_get_impl(loop));
#if defined(WIN32) || defined(WIN64)
    WSACleanup();
#endif /* defined(WIN32) || defined(WIN64) */
}

int _select(loop_t* loop) {
    socket_t max_fd = 0;
    socket_t fd = 0;
    int error = 0;
    dlist_node_t* node = 0;
    dlist_node_t* temp = 0;
    channel_ref_t* channel_ref = 0;
    struct timeval tv = {0, 100}; /* 空转时最多等待1ms */
    loop_select_t* impl = (loop_select_t*)loop_get_impl(loop);
    FD_ZERO(impl->read_fds);
    FD_ZERO(impl->send_fds);
    dlist_for_each_safe(loop_get_active_list(loop), node, temp) {
        channel_ref = (channel_ref_t*)dlist_node_get_data(node);
        fd = channel_ref_get_socket_fd(channel_ref);
        if (channel_ref_check_balance(channel_ref)) {
        }
        if (channel_ref_check_event(channel_ref, channel_event_recv)) {
            FD_SET(fd, impl->read_fds);
            max_fd = max(max_fd, fd);
        }
        if (channel_ref_check_event(channel_ref, channel_event_send)) {
            FD_SET(fd, impl->send_fds);
            max_fd = max(max_fd, fd);
        }
    }
    error = select(max_fd + 1, impl->read_fds, impl->send_fds, 0, &tv);
    if (0 > error) {
        return error_loop_fail;
    }
    return error_ok;
}

int impl_run_once(loop_t* loop) {
    dlist_node_t* node = 0;
    dlist_node_t* temp = 0;
    channel_ref_t* channel_ref = 0;
    socket_t fd = 0;
    time_t ts = time(0);
    loop_select_t* impl = (loop_select_t*)loop_get_impl(loop);
    int error = _select(loop);
    if (error != error_ok) {
        return error;
    }
    dlist_for_each_safe(loop_get_active_list(loop), node, temp) {
        channel_ref = (channel_ref_t*)dlist_node_get_data(node);
        fd = channel_ref_get_socket_fd(channel_ref);
        if (FD_ISSET(fd, impl->read_fds)) {
            channel_ref_update(channel_ref, channel_event_recv, ts);
        }
        if (FD_ISSET(fd, impl->send_fds)) {
            channel_ref_update(channel_ref, channel_event_send, ts);
        }
    }
    loop_check_timeout(loop, ts);
    loop_check_close(loop);
    return error_ok;
}

socket_t impl_channel_accept(channel_ref_t* channel_ref) {
    channel_ref;
    return 0;
}

int impl_event_add(channel_ref_t* channel_ref, channel_event_e e) {
    channel_ref;
    e;
    return error_ok;
}

int impl_event_remove(channel_ref_t* channel_ref, channel_event_e e) {
    channel_ref;
    e;
    return error_ok;
}

int impl_add_channel_ref(loop_t* loop, channel_ref_t* channel_ref) {
    loop;
    channel_ref;
    return error_ok;
}

int impl_remove_channel_ref(loop_t* loop, channel_ref_t* channel_ref) {
    loop;
    channel_ref;
    return error_ok;
}

#endif
