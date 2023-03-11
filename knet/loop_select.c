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

#ifdef LOOP_SELECT

#include "loop.h"
#include "list.h"
#include "channel_ref.h"
#include "logger.h"
#include "misc.h"
#include "hash.h"

typedef struct _loop_select_t {
    fd_set read_fds; /* select读描述符数组 */
    fd_set send_fds; /* select写描述符数组 */
    socket_t max_fd;
    khash_t* hash;
} loop_select_t;

typedef struct _fd_info {
    int read;
    int write;
    kchannel_ref_t* channel_ref;
} fd_info;

void hash_dtor(void* v) {
    knet_free(v);
}

int knet_impl_create(kloop_t* loop) {
#if defined(_WIN32) || defined(WIN64)
    WSADATA wsa;
#endif /* defined(_WIN32) || defined(WIN64) */
    loop_select_t* impl = knet_create(loop_select_t);
    knet_loop_set_impl(loop, impl);
#if defined(_WIN32) || defined(WIN64)
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif /* defined(_WIN32) || defined(WIN64) */
    FD_ZERO(&impl->read_fds);
    FD_ZERO(&impl->send_fds);
    impl->max_fd = 0;
    impl->hash = hash_create(FD_SETSIZE, hash_dtor);
    return error_ok;
}

void knet_impl_destroy(kloop_t* loop) {
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    hash_destroy(impl->hash);
    knet_free(impl);
#if defined(_WIN32) || defined(WIN64)
    WSACleanup();
#endif /* defined(_WIN32) || defined(WIN64) */
}

int _select(kloop_t* loop) {
    int error = 0;
    struct timeval tv = {0, 1000}; /* 空转时最多等待1ms */
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    khash_value_t* value = 0;
    fd_info* info = 0;
    socket_t fd = 0;
    FD_ZERO(&impl->read_fds);
    FD_ZERO(&impl->send_fds);
    impl->max_fd = 0;
    hash_for_each_safe(impl->hash, value) {
        fd = (socket_t)hash_value_get_key(value);
        info = (fd_info*)hash_value_get_value(value);
        impl->max_fd = impl->max_fd > fd ? impl->max_fd : fd;
        if (info->read) {
            FD_SET(fd, &impl->read_fds);
        }
        if (info->write) {
            FD_SET(fd, &impl->send_fds);
        }
    }
    if (impl->max_fd) {
        error = select((int)impl->max_fd + 1, &impl->read_fds, &impl->send_fds, 0, &tv);
        if (0 > error) {
            return error_loop_fail;
        }
    } else {
        thread_sleep_ms(1);
    }
    return error_ok;
}

int knet_impl_run_once(kloop_t* loop) {
    khash_value_t* value = 0;
    fd_info* info = 0;
    socket_t fd = 0;
    time_t ts = time(0);
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    int error = _select(loop);
    if (error != error_ok) {
        return error;
    }
    hash_for_each_safe(impl->hash, value) {
        fd = (socket_t)hash_value_get_key(value);
        info = (fd_info*)hash_value_get_value(value);
        fd = knet_channel_ref_get_socket_fd(info->channel_ref);
        if (info->read && FD_ISSET(fd, &impl->read_fds)) {
            knet_channel_ref_update(info->channel_ref, channel_event_recv, ts);
        }
        if (info->write && FD_ISSET(fd, &impl->send_fds)) {
            knet_channel_ref_update(info->channel_ref, channel_event_send, ts);
        }
    }
    knet_loop_check_timeout(loop, ts);
    knet_loop_check_close(loop);
    return error_ok;
}

socket_t knet_impl_channel_accept(kchannel_ref_t* channel_ref) {
    channel_ref;
    return 0;
}

int knet_impl_event_add(kchannel_ref_t* channel_ref, knet_channel_event_e e) {
    socket_t fd = knet_channel_ref_get_socket_fd(channel_ref);
    kloop_t* loop = knet_channel_ref_get_loop(channel_ref);
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    fd_info* info = (fd_info*)hash_get(impl->hash, (uint32_t)fd);
    if (e & channel_event_recv) {
        info->read = 1;
    }
    if (e & channel_event_send) {
        info->write = 1;
    }
    return error_ok;
}

int knet_impl_event_remove(kchannel_ref_t* channel_ref, knet_channel_event_e e) {
    socket_t fd = knet_channel_ref_get_socket_fd(channel_ref);
    kloop_t* loop = knet_channel_ref_get_loop(channel_ref);
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    fd_info* info = (fd_info*)hash_get(impl->hash, (uint32_t)fd);
    if (e & channel_event_recv) {
        info->read = 0;
    }
    if (e & channel_event_send) {
        info->write = 0;
    }
    return error_ok;
}

int knet_impl_add_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    fd_info* info = knet_malloc(sizeof(fd_info));
    info->read = 0;
    info->write = 0;
    info->channel_ref = channel_ref;
    hash_add(impl->hash, (uint32_t)knet_channel_ref_get_socket_fd(channel_ref), (void*)info);
    return error_ok;
}

int knet_impl_remove_channel_ref(kloop_t* loop, kchannel_ref_t* channel_ref) {
    loop_select_t* impl = (loop_select_t*)knet_loop_get_impl(loop);
    hash_delete(impl->hash, (uint32_t)knet_channel_ref_get_socket_fd(channel_ref));
    return error_ok;
}

#endif
