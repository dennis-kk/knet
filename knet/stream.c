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

#include "stream.h"
#include "config.h"
#include "channel_ref.h"
#include "ringbuffer.h"
#include "logger.h"
#include "buffer.h"
#include "misc.h"

struct _stream_t {
    channel_ref_t* channel_ref;
};

stream_t* stream_create(channel_ref_t* channel_ref) {
    stream_t* stream = 0;
    assert(channel_ref);
    stream = create(stream_t);
    memset(stream, 0, sizeof(stream_t));
    assert(stream);
    stream->channel_ref = channel_ref;
    return stream;
}

void stream_destroy(stream_t* stream) {
    assert(stream);
    destroy(stream);
}

int stream_available(stream_t* stream) {
    assert(stream);
    return ringbuffer_available(channel_ref_get_ringbuffer(stream->channel_ref));
}

int stream_pop(stream_t* stream, void* buffer, int size) {
    assert(stream);
    assert(buffer);
    assert(size);
    if (0 < ringbuffer_read(channel_ref_get_ringbuffer(stream->channel_ref), (char*)buffer, size)) {
        return error_ok;
    }
    return error_recv_fail;
}

int stream_eat_all(stream_t* stream) {
    assert(stream);
    return ringbuffer_eat_all(channel_ref_get_ringbuffer(stream->channel_ref));
}

int stream_eat(stream_t* stream, int size) {
    assert(stream);
    if (!size) {
        return error_ok;
    }
    return ringbuffer_eat(channel_ref_get_ringbuffer(stream->channel_ref), size);
}

int stream_push(stream_t* stream, void* buffer, int size) {
    assert(stream);
    assert(buffer);
    assert(size);
    return channel_ref_write(stream->channel_ref, (char*)buffer, size);
}

int stream_copy(stream_t* stream, void* buffer, int size) {
    assert(stream);
    assert(buffer);
    assert(size);
    if (0 > ringbuffer_copy(channel_ref_get_ringbuffer(stream->channel_ref), (char*)buffer, size)) {
        return error_recv_fail;
    }
    return error_ok;
}
