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

#include <stdlib.h>
#include "ringbuffer.h"
#include "logger.h"

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

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
#ifdef DISABLE_KNET_MEM_FUNC
    kringbuffer_t* rb = (kringbuffer_t*)malloc(sizeof(kringbuffer_t));
#else
    kringbuffer_t* rb = knet_create(kringbuffer_t);
#endif
    verify(rb);
    memset(rb, 0, sizeof(kringbuffer_t));
    rb->lock_type = 0;
    rb->max_size  = size;
#ifdef DISABLE_KNET_MEM_FUNC
    rb->ptr       = (char*)malloc(size);
#else
    rb->ptr       = knet_create_raw(size);
#endif
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

uint32_t ringbuffer_remove(kringbuffer_t* rb, uint32_t size) {
    uint32_t i = 0;
    verify(rb);
    verify(size);
    size = min(rb->count, size);
    for (; i < size; i++) {
        rb->read_pos = (rb->read_pos + 1) % rb->max_size;
    }
    rb->count -= size;
    return size;
}

void ringbuffer_enlarge(kringbuffer_t* rb, uint32_t size) {
    uint32_t reserve_size = rb->max_size - rb->count;
    uint32_t cur_count = rb->count;
    uint32_t new_max_size = 0;
    char* new_ptr = 0;
    char* temp_ptr = 0;
    verify(rb);
    verify(size);
    if (reserve_size < size) {
        new_max_size = rb->max_size * 2;
        while (new_max_size - cur_count < size) {
            new_max_size += rb->max_size;
        }
#ifdef DISABLE_KNET_MEM_FUNC
        new_ptr = malloc(new_max_size);
#else
        new_ptr = knet_malloc(new_max_size);
#endif
        temp_ptr = rb->ptr;
        if (cur_count) {
          ringbuffer_read(rb, new_ptr, cur_count);
        } else {
          ringbuffer_read(rb, new_ptr, rb->max_size);
        }
        memset(rb, 0, sizeof(struct _ringbuffer_t));
        rb->ptr = new_ptr;
        rb->read_pos = 0;
        rb->write_pos = cur_count;
        rb->max_size = new_max_size;
        rb->count = cur_count;
    #ifdef DISABLE_KNET_MEM_FUNC
        free(temp_ptr);
    #else
        knet_free(temp_ptr);
    #endif
    } else {
        return;
    }
}

uint32_t ringbuffer_write(kringbuffer_t* rb, const char* buffer, uint32_t size) {
    uint32_t i = 0;
    verify(rb);
    verify(buffer);
    verify(size);
    ringbuffer_enlarge(rb, size);
    size = (((rb->max_size - rb->count) > size) ? size : rb->max_size - rb->count);
    for (; i < size; i++) {
        rb->ptr[rb->write_pos] = buffer[i];
        rb->write_pos = (rb->write_pos + 1) % rb->max_size;
    }
    rb->count += size;
    return size;
}

uint32_t ringbuffer_replace(kringbuffer_t* rb, uint32_t pos, const char* buffer, uint32_t size) {
    uint32_t i         = 0;
    uint32_t write_pos = 0;
    verify(rb);
    verify(buffer);
    verify(size);
    if (size > rb->max_size) {
        return 0;
    }
    write_pos = (rb->read_pos + pos) % rb->max_size;
    for (; i < size; i++) {
        rb->ptr[write_pos] = buffer[i];
        write_pos = (write_pos + 1) % rb->max_size;
    }
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

uint32_t ringbuffer_copy_random(kringbuffer_t* rb, uint32_t pos, char* buffer, uint32_t size) {
    uint32_t i = 0;
    uint32_t read_pos = 0;
    verify(rb);
    verify(pos >= 0);
    verify(size);
    verify(buffer);
    if (pos + size > rb->count) {
        return 0;
    }
    size = min(rb->count - pos, size);
    read_pos = (rb->read_pos + pos) % rb->max_size;
    for (; i < size; i++) {
        buffer[i] = rb->ptr[read_pos];
        read_pos = (read_pos + 1) % rb->max_size;
    }
    return size;
}

uint32_t ringbuffer_find(kringbuffer_t* rb, const char* target, uint32_t* size) {
    uint32_t i      = 0; /* 是否达到末尾 */
    int      index  = 0; /* 当前匹配到的字符下标 */
    int      length = 0; /* 总长度 */
    int      pos    = 0; /* 在rb内的下标 */
    verify(rb);
    verify(target);
    verify(size);
    length = (int)strlen(target);
    pos    = rb->read_pos % rb->max_size;
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
#ifdef DISABLE_KNET_MEM_FUNC
    free(rb->ptr);
    free(rb);
#else
    knet_free(rb->ptr);
    knet_free(rb);
#endif
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

void ringbuffer_read_unlock(kringbuffer_t* rb) {
    verify(rb);
    rb->lock_size = 0;
    rb->lock_type = 0;
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
        ringbuffer_enlarge(rb, rb->max_size);
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

void ringbuffer_write_unlock(kringbuffer_t* rb) {
    verify(rb);
    rb->lock_size = 0;
    rb->lock_type = 0;
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

void ringbuffer_print_stdout(kringbuffer_t* rb) {
    uint32_t i   = 0;
    uint32_t pos = 0;
    verify(rb);
    pos = rb->read_pos;
    fprintf(stdout, "MaxSize: %u, Count: %u\n", rb->max_size, rb->count);
    for (; i < rb->count; i++) {
        fprintf(stdout, "%c", rb->ptr[pos]);
        pos = (pos + 1) % rb->max_size;
    }
}
