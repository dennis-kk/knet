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
#include <stdarg.h>
#include "stream.h"
#include "config.h"
#include "channel_ref.h"
#include "ringbuffer.h"

#include "buffer.h"
#include "misc.h"

struct _stream_t {
    kchannel_ref_t* channel_ref;
};

kstream_t* stream_create(kchannel_ref_t* channel_ref) {
    kstream_t* stream = 0;
    verify(channel_ref);
    stream = create(kstream_t);
    memset(stream, 0, sizeof(kstream_t));
    verify(stream);
    stream->channel_ref = channel_ref;
    return stream;
}

void stream_destroy(kstream_t* stream) {
    verify(stream);
    destroy(stream);
}

int knet_stream_available(kstream_t* stream) {
    verify(stream);
    return ringbuffer_available(knet_channel_ref_get_ringbuffer(stream->channel_ref));
}

int knet_stream_pop(kstream_t* stream, void* buffer, int size) {
    verify(stream);
    verify(buffer);
    verify(size);
    if (0 < ringbuffer_read(knet_channel_ref_get_ringbuffer(stream->channel_ref), (char*)buffer, size)) {
        return error_ok;
    }
    return error_recv_fail;
}

int knet_stream_eat_all(kstream_t* stream) {
    verify(stream);
    return ringbuffer_eat_all(knet_channel_ref_get_ringbuffer(stream->channel_ref));
}

int knet_stream_eat(kstream_t* stream, int size) {
    verify(stream);
    if (!size) {
        return error_ok;
    }
    return ringbuffer_eat(knet_channel_ref_get_ringbuffer(stream->channel_ref), size);
}

int knet_stream_push(kstream_t* stream, const void* buffer, int size) {
    verify(stream);
    verify(buffer);
    verify(size);
    return knet_channel_ref_write(stream->channel_ref, (char*)buffer, size);
}

int knet_stream_push_varg(kstream_t* stream, const char* format, ...) {
    char buffer[1024] = {0};
    int len           = 0;
    va_list arg_ptr;
    verify(stream);
    verify(format);
    va_start(arg_ptr, format);
    #if defined(WIN32)
    len = _vsnprintf(buffer, sizeof(buffer), format, arg_ptr);
    if ((len >= sizeof(buffer)) || (len < 0)) {
        return error_stream_buffer_overflow;
    }
    #else
    len = vsnprintf(buffer, sizeof(buffer), format, arg_ptr);
    if (len <= 0) {
        return error_stream_buffer_overflow;
    }
    #endif /* defined(WIN32) */
    va_end(arg_ptr);
    return knet_stream_push(stream, buffer, len);
}

int knet_stream_copy(kstream_t* stream, void* buffer, int size) {
    verify(stream);
    verify(buffer);
    verify(size);
    if (0 >= ringbuffer_copy(knet_channel_ref_get_ringbuffer(stream->channel_ref), (char*)buffer, size)) {
        return error_recv_fail;
    }
    return error_ok;
}

int knet_stream_push_stream(kstream_t* stream, kstream_t* target) {
    uint32_t      size = 0;
    kringbuffer_t* rb   = 0;
    char*         ptr  = 0;
    verify(stream);
    verify(target);
    if (stream == target) {
        return error_send_fail;
    }
    rb = knet_channel_ref_get_ringbuffer(stream->channel_ref);
    verify(rb);
    /* 从ringbuffer内取数据写入另一个stream */
    for (size = ringbuffer_read_lock_size(rb);
        (size);
        ringbuffer_read_commit(rb, size), size = ringbuffer_read_lock_size(rb)) {
        ptr = ringbuffer_read_lock_ptr(rb);
        verify(ptr);
        if (error_ok != knet_stream_push(target, ptr, size)) {
            ringbuffer_read_commit(rb, size);
            return error_send_fail;
        }
    }
    return error_ok;
}

int knet_stream_copy_stream(kstream_t* stream, kstream_t* target) {
    uint32_t      size = 0;
    kringbuffer_t* rb   = 0;
    char*         ptr  = 0;
    verify(stream);
    verify(target);
    if (stream == target) {
        return error_send_fail;
    }
    rb = knet_channel_ref_get_ringbuffer(stream->channel_ref);
    verify(rb);
    /* 从ringbuffer内取数据写入另一个stream，使用虚拟窗口，不清除数据 */
    for (size = ringbuffer_window_read_lock_size(rb);
        (size);
        ringbuffer_window_read_commit(rb, size), size = ringbuffer_window_read_lock_size(rb)) {
        ptr = ringbuffer_window_read_lock_ptr(rb);
        verify(ptr);
        if (error_ok != knet_stream_push(target, ptr, size)) {
            ringbuffer_window_read_commit(rb, size);
            return error_send_fail;
        }
    }
    return error_ok;
}

kchannel_ref_t* knet_stream_get_channel_ref(kstream_t* stream) {
    verify(stream);
    return stream->channel_ref;
}
