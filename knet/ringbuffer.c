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

#include <stdlib.h>
#include "ringbuffer.h"
#include "logger.h"

struct _ringbuffer_t {
    char*    ptr;       /* 缓冲区指针 */
    uint32_t read_pos;  /* 读索引 */
    uint32_t write_pos; /* 写索引 */
    uint32_t max_size;  /* 最大长度 */
    uint32_t lock_size; /* 锁定长度 */
    uint32_t lock_type; /* 锁定类型， 1： 读锁定  2： 写锁定 */
    uint32_t count;     /* 可读数据长度 */
};

ringbuffer_t* ringbuffer_create(uint32_t size) {
    ringbuffer_t* rb = create(ringbuffer_t);
    assert(rb);
    rb->lock_type = 0;
    rb->max_size  = size;
    rb->ptr       = create_raw(size);
    assert(rb->ptr);
    rb->lock_size = 0;
    rb->read_pos  = 0;
    rb->write_pos = 0;
    rb->count     = 0;
    return rb;
}

int ringbuffer_eat_all(ringbuffer_t* rb) {
    assert(rb);
    if (rb->lock_size || rb->lock_type) {
        return error_recvbuffer_locked;
    }
    rb->lock_size = 0;
    rb->read_pos  = 0;
    rb->write_pos = 0;
    rb->count     = 0;
    return error_ok;
}

int ringbuffer_eat(ringbuffer_t* rb, uint32_t size) {
    assert(rb);
    if (rb->lock_size || rb->lock_type) {
        return error_recvbuffer_locked;
    }
    if (size > rb->count) {
        return error_recvbuffer_not_enough;
    }
    rb->count -= size;
    rb->read_pos = (rb->read_pos + size) % rb->max_size;
    return error_ok;
}

uint32_t ringbuffer_read(ringbuffer_t* rb, char* buffer, uint32_t size) {
    uint32_t i = 0;
    assert(rb);
    assert(buffer);
    assert(size);
    size = min(rb->count, size);
    for (; i < size; i++) {
        buffer[i] = rb->ptr[rb->read_pos];
        rb->read_pos = (rb->read_pos + 1) % rb->max_size;
    }
    rb->count -= size;
    return size;
}

uint32_t ringbuffer_copy(ringbuffer_t* rb, char* buffer, uint32_t size) {
    uint32_t i        = 0;
    uint32_t read_pos = rb->read_pos;
    assert(rb);
    assert(buffer);
    assert(size);
    size = min(rb->count, size);
    for (; i < size; i++) {
        buffer[i] = rb->ptr[read_pos];
        read_pos = (read_pos + 1) % rb->max_size;
    }
    return size;
}

uint32_t ringbuffer_available(ringbuffer_t* rb) {
    assert(rb);
    return rb->count;
}

void ringbuffer_destroy(ringbuffer_t* rb) {
    assert(rb);
    destroy(rb->ptr);
    destroy(rb);
}

uint32_t ringbuffer_read_lock_size(ringbuffer_t* rb) {
    assert(rb);
    if (ringbuffer_empty(rb)) {
        return 0;
    }
    rb->lock_type = 1;
    rb->lock_size = 0;
    if (rb->write_pos > rb->read_pos) {
        rb->lock_size = rb->write_pos - rb->read_pos;
    } else {
        rb->lock_size = rb->max_size - rb->read_pos;
    }
    return rb->lock_size;
}

char* ringbuffer_read_lock_ptr(ringbuffer_t* rb) {
    assert(rb);
    if (rb->lock_type != 1) {
        return 0;
    }
    return rb->ptr + rb->read_pos;
}

void ringbuffer_read_commit(ringbuffer_t* rb, uint32_t size) {
    assert(rb);
    if (rb->lock_type != 1) {
        return;
    }
    if (rb->lock_size < size) {
        return;
    }
    rb->read_pos = (rb->read_pos + size) % rb->max_size;
    rb->lock_size = 0;
    rb->lock_type = 0;
    rb->count -= size;
}

uint32_t ringbuffer_write_lock_size(ringbuffer_t* rb) {
    assert(rb);
    if (ringbuffer_full(rb)) {
        return 0;
    }
    rb->lock_type = 2;
    rb->lock_size = 0;
    if (rb->write_pos >= rb->read_pos) {
        rb->lock_size = rb->max_size - rb->write_pos;
    } else {
        rb->lock_size = rb->read_pos - rb->write_pos;
    }
    return rb->lock_size;
}

char* ringbuffer_write_lock_ptr(ringbuffer_t* rb) {
    assert(rb);
    if (rb->lock_type != 2) {
        return 0;
    }
    return rb->ptr + rb->write_pos;
}

void ringbuffer_write_commit(ringbuffer_t* rb, uint32_t size) {
    assert(rb);
    if (rb->lock_type != 2) {
        return;
    }
    if (rb->lock_size < size) {
        return;
    }
    rb->write_pos = (rb->write_pos + size) % rb->max_size;
    rb->lock_size = 0;
    rb->lock_type = 0;
    rb->count += size;
}

int ringbuffer_full(ringbuffer_t* rb) {
    assert(rb);
    return (rb->count == rb->max_size);
}

int ringbuffer_empty(ringbuffer_t* rb) {
    assert(rb);
    return !(rb->count);
}

uint32_t ringbuffer_get_max_size(ringbuffer_t* rb) {
    assert(rb);
    return rb->max_size;
}
