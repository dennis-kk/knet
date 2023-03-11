/*
 * Copyright (c) 2014-2016, dennis wang
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

#include "loop_profile.h"
#include "loop.h"
#include "list.h"
#include "stream.h"
#include "logger.h"

struct _loop_profile_t {
    kloop_t*  loop;                /* 网络事件循环 */
    uint64_t recv_bytes;          /* 已接收的字节数 */
    uint64_t send_bytes;          /* 已发送的字节数 */
    uint32_t established_channel; /* 已经建立连接的管道数量 */
    uint32_t active_channel;      /* 还未建立连接的管道数量 */
    uint32_t close_channel;       /* 已关闭的管道数量 */
    uint32_t __padding;           /* 填充 */
    uint64_t last_send_bytes;     /* 上次调用knet_loop_profile_get_sent_bandwidth时的发送字节数 */
    uint64_t last_recv_bytes;     /* 上次调用knet_loop_profile_get_recv_bandwidth时的接收字节数 */
    time_t   last_send_tick;      /* 上次调用knet_loop_profile_get_sent_bandwidth时的时间戳（秒） */
    time_t   last_recv_tick;      /* 上次调用knet_loop_profile_get_recv_bandwidth时的时间戳（秒） */
};

kloop_profile_t* knet_loop_profile_create(kloop_t* loop) {
    kloop_profile_t* profile = 0;
    verify(loop);
    profile = knet_create(kloop_profile_t);
    verify(profile);
    memset(profile, 0, sizeof(kloop_profile_t));
    profile->loop           = loop;
    profile->last_send_tick = time(0);
    profile->last_recv_tick = profile->last_send_tick;
    return profile;
}

void knet_loop_profile_destroy(kloop_profile_t* profile) {
    knet_free(profile);
}

uint32_t knet_loop_profile_increase_established_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return ++profile->established_channel;
}

uint32_t knet_loop_profile_decrease_established_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return --profile->established_channel;
}

uint32_t knet_loop_profile_get_established_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return profile->established_channel - 2; /* 事件通知管道不计入统计数据 */
}

uint32_t knet_loop_profile_increase_active_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return ++profile->active_channel;
}

uint32_t knet_loop_profile_decrease_active_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return --profile->active_channel;
}

uint32_t knet_loop_profile_get_active_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return profile->active_channel;
}

uint32_t knet_loop_profile_increase_close_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return ++profile->close_channel;
}

uint32_t knet_loop_profile_decrease_close_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return --profile->close_channel;
}

uint32_t knet_loop_profile_get_close_channel_count(kloop_profile_t* profile) {
    verify(profile);
    return profile->close_channel;
}

uint64_t knet_loop_profile_add_send_bytes(kloop_profile_t* profile, uint64_t send_bytes) {
    verify(profile);
    return (profile->send_bytes += send_bytes);
}

uint64_t knet_loop_profile_get_sent_bytes(kloop_profile_t* profile) {
    verify(profile);
    return profile->send_bytes;
}

uint64_t knet_loop_profile_add_recv_bytes(kloop_profile_t* profile, uint64_t recv_bytes) {
    verify(profile);
    return (profile->recv_bytes += recv_bytes);
}

uint64_t knet_loop_profile_get_recv_bytes(kloop_profile_t* profile) {
    verify(profile);
    return profile->recv_bytes;
}

uint32_t knet_loop_profile_get_sent_bandwidth(kloop_profile_t* profile) {
    time_t   tick      = time(0);
    uint64_t bandwidth = 0;
    uint64_t intval    = 0;
    uint64_t bytes     = 0;
    verify(profile);
    bytes = profile->send_bytes - profile->last_send_bytes;
    if (tick == profile->last_send_tick) {
        /* 最小为1秒 */
        intval = 1;
    } else {
        intval = tick - profile->last_send_tick;
    }
    bandwidth = bytes / intval;
    profile->last_send_tick  = tick;
    profile->last_send_bytes = profile->send_bytes;
    return (uint32_t)bandwidth;
}

uint32_t knet_loop_profile_get_recv_bandwidth(kloop_profile_t* profile) {
    time_t   tick      = time(0);
    uint64_t bandwidth = 0;
    uint64_t intval    = 0;
    uint64_t bytes     = 0;
    verify(profile);
    bytes = profile->recv_bytes - profile->last_recv_bytes;
    if (tick == profile->last_recv_tick) {
        /* 最小为1秒 */
        intval = 1;
    } else {
        intval = tick - profile->last_recv_tick;
    }
    bandwidth = bytes / intval;
    profile->last_recv_tick  = tick;
    profile->last_recv_bytes = profile->recv_bytes;
    return (uint32_t)bandwidth;
}

int knet_loop_profile_dump_file(kloop_profile_t* profile, FILE* fp) {
    int len = 0;
    verify(profile);
    verify(fp);
    len = fprintf(
        fp,
        "Established channel: %ld\n"
        "Active channel:      %ld\n"
        "Close channel:       %ld\n"
        "Received bytes:      %lld\n"
        "Sent bytes:          %lld\n"
        "Received bandwidth:  %ld(B/s)\n"
        "Sent bandwidth:      %ld(B/s)\n",
        (long)knet_loop_profile_get_established_channel_count(profile),
        (long)knet_loop_profile_get_active_channel_count(profile),
        (long)knet_loop_profile_get_close_channel_count(profile),
        (long long)knet_loop_profile_get_recv_bytes(profile),
        (long long)knet_loop_profile_get_sent_bytes(profile),
        (long)knet_loop_profile_get_recv_bandwidth(profile),
        (long)knet_loop_profile_get_sent_bandwidth(profile));
    if (len <= 0) {
        return error_fail;
    }
    return error_ok;
}

int knet_loop_profile_dump_stream(kloop_profile_t* profile, kstream_t* stream) {
    verify(profile);
    verify(stream);
    return knet_stream_push_varg(
        stream,
        "Established channel: %ld\n"
        "Active channel:      %ld\n"
        "Close channel:       %ld\n"
        "Received bytes:      %lld\n"
        "Sent bytes:          %lld\n"
        "Received bandwidth:  %ld(B/s)\n"
        "Sent bandwidth:      %ld(B/s)\n",
        (long)knet_loop_profile_get_established_channel_count(profile),
        (long)knet_loop_profile_get_active_channel_count(profile),
        (long)knet_loop_profile_get_close_channel_count(profile),
        (long long)knet_loop_profile_get_recv_bytes(profile),
        (long long)knet_loop_profile_get_sent_bytes(profile),
        (long)knet_loop_profile_get_recv_bandwidth(profile),
        (long)knet_loop_profile_get_sent_bandwidth(profile));
}

int knet_loop_profile_dump_stdout(kloop_profile_t* profile) {
    int len = 0;
    verify(profile);
    len = fprintf(
        stdout,
        "Established channel: %ld\n"
        "Active channel:      %ld\n"
        "Close channel:       %ld\n"
        "Received bytes:      %lld\n"
        "Sent bytes:          %lld\n"
        "Received bandwidth:  %ld(B/s)\n"
        "Sent bandwidth:      %ld(B/s)\n",
        (long)knet_loop_profile_get_established_channel_count(profile),
        (long)knet_loop_profile_get_active_channel_count(profile),
        (long)knet_loop_profile_get_close_channel_count(profile),
        (long long)knet_loop_profile_get_recv_bytes(profile),
        (long long)knet_loop_profile_get_sent_bytes(profile),
        (long)knet_loop_profile_get_recv_bandwidth(profile),
        (long)knet_loop_profile_get_sent_bandwidth(profile));
    if (len <= 0) {
        return error_fail;
    }
    return error_ok;
}
