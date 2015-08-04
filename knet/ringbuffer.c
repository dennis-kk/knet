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

#include <stdlib.h>
#include "ringbuffer.h"


struct _ringbuffer_t {
    char*    ptr;                   /* 缓冲区指针 */
    uint32_t read_pos;              /* 读索引 */
    uint32_t write_pos;             /* 写索引 */
    uint32_t max_size;              /* 最大长度 */
    uint32_t lock_size;             /* 锁定长度 */
    uint32_t lock_type;             /* 锁定类型， 1： 读锁定  2： 写锁定 */
    uint32_t count;                 /* 可读数据长度 */
    uint32_t window_read_lock_size; /* 窗口读锁定长度 */
    uint32_t window_read_pos;       /* 窗口读位置 */
};

kringbuffer_t* ringbuffer_create(uint32_t size) {
    kringbuffer_t* rb = create(kringbuffer_t);
    verify(rb);
    memset(rb, 0, sizeof(kringbuffer_t));
    rb->lock_type = 0;
    rb->max_size  = size;
    rb->ptr       = create_raw(size);
    verify(rb->ptr);
    rb->lock_size = 0;
    rb->read_pos  = 0;
    rb->write_pos = 0;
    rb->count     = 0;
    return rb;
}

int ringbuffer_eat_all(kringbuffer_t* rb) {
    verify(rb);
    if (rb->lock_size || rb->lock_type) {
        return error_recvbuffer_locked;
    }
    rb->lock_size = 0;
    rb->read_pos  = 0;
    rb->write_pos = 0;
    rb->count     = 0;
    return error_ok;
}

int ringbuffer_eat(kringbuffer_t* rb, uint32_t size) {
    verify(rb);
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

uint32_t ringbuffer_read(kringbuffer_t* rb, char* buffer, uint32_t size) {
    uint32_t i = 0;
    verify(rb);
    verify(buffer);
    verify(size);
    size = min(rb->count, size);
    for (; i < size; i++) {
        buffer[i] = rb->ptr[rb->read_pos];
        rb->read_pos = (rb->read_pos + 1) % rb->max_size;
    }
    rb->count -= size;
    return size;
}

uint32_t ringbuffer_copy(kringbuffer_t* rb, char* buffer, uint32_t size) {
    uint32_t i        = 0;
    uint32_t read_pos = 0;
    verify(rb);
    verify(buffer);
    verify(size);
    read_pos = rb->read_pos;
    size = min(rb->count, size);
    for (; i < size; i++) {
        buffer[i] = rb->ptr[read_pos];
        read_pos = (read_pos + 1) % rb->max_size;
    }
    return size;
}

uint32_t ringbuffer_find(kringbuffer_t* rb, const char* target, uint32_t* size) {
    uint32_t i      = 0;                           /* 是否达到末尾 */
    int      index  = 0;                           /* 当前匹配到的字符下标 */
    int      length = strlen(target);              /* 总长度 */
    int      pos    = rb->read_pos % rb->max_size; /* 在rb内的下标 */
    verify(rb);
    verify(target);
    verify(size);
    for (; (i < rb->count) && (index < length); i++) {
        if (rb->ptr[pos] == target[index]) { /* 匹配 */
            index += 1;                        /* 当前位置递增 */
            pos    = (pos + 1) % rb->max_size; /* rb读位置递增 */
        } else { /* 失配 */
            pos   = (rb->read_pos + i + 1) % rb->max_size; /* 重新设置rb匹配位置 */
            index = 0;                                 /* 重置目标字符串下标 */
        }
    }
    if (index == length) {
        *size = i; /* 搜索所经历的所有字符数，包含target */
        return error_ok;
    }
    return error_ringbuffer_not_found;
}

uint32_t ringbuffer_available(kringbuffer_t* rb) {
    verify(rb);
    return rb->count;
}

void ringbuffer_destroy(kringbuffer_t* rb) {
    verify(rb);
    destroy(rb->ptr);
    destroy(rb);
}

uint32_t ringbuffer_read_lock_size(kringbuffer_t* rb) {
    verify(rb);
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

char* ringbuffer_read_lock_ptr(kringbuffer_t* rb) {
    verify(rb);
    if (rb->lock_type != 1) {
        return 0;
    }
    return rb->ptr + rb->read_pos;
}

void ringbuffer_read_commit(kringbuffer_t* rb, uint32_t size) {
    verify(rb);
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

uint32_t ringbuffer_window_read_lock_size(kringbuffer_t* rb) {
    verify(rb);
    if (ringbuffer_empty(rb)) {
        return 0;
    }
    if (rb->window_read_pos == rb->write_pos) {
        rb->window_read_pos = 0;
        return 0;
    }
    if (!rb->window_read_pos) {
        rb->window_read_pos = rb->read_pos;
    }
    rb->window_read_lock_size = 0;
    if (rb->write_pos > rb->window_read_pos) {
        rb->window_read_lock_size = rb->write_pos - rb->window_read_pos;
    } else {
        rb->window_read_lock_size = rb->max_size - rb->window_read_pos;
    }
    return rb->window_read_lock_size;
}

char* ringbuffer_window_read_lock_ptr(kringbuffer_t* rb) {
    verify(rb);
    return rb->ptr + rb->window_read_pos;
}

void ringbuffer_window_read_commit(kringbuffer_t* rb, uint32_t size) {
    verify(rb);
    verify(size);
    if (rb->window_read_lock_size < size) {
        return;
    }
    rb->window_read_pos = (rb->window_read_pos + size) % rb->max_size;
}

uint32_t ringbuffer_write_lock_size(kringbuffer_t* rb) {
    verify(rb);
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

char* ringbuffer_write_lock_ptr(kringbuffer_t* rb) {
    verify(rb);
    if (rb->lock_type != 2) {
        return 0;
    }
    return rb->ptr + rb->write_pos;
}

void ringbuffer_write_commit(kringbuffer_t* rb, uint32_t size) {
    verify(rb);
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

int ringbuffer_full(kringbuffer_t* rb) {
    verify(rb);
    return (rb->count == rb->max_size);
}

int ringbuffer_empty(kringbuffer_t* rb) {
    verify(rb);
    return !(rb->count);
}

uint32_t ringbuffer_get_max_size(kringbuffer_t* rb) {
    verify(rb);
    return rb->max_size;
}
