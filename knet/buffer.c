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

#include "buffer.h"
#include "logger.h"

/**
 * 发送缓冲区
 */
struct _buffer_t {
    char*    ptr; /* 缓冲区起始地址 */
    uint32_t len; /* 缓冲区长度 */
    uint32_t pos; /* 缓冲区当前位置 */
};

kbuffer_t* knet_buffer_create(uint32_t size) {
    kbuffer_t* sb = create(kbuffer_t);
    verify(sb);
    if (!sb) {
        return 0;
    }
    /* 数据指针 */
    sb->ptr = create_raw(size);
    verify(sb->ptr);
    if (!sb->ptr) {
        knet_buffer_destroy(sb);
        return 0;
    }
    sb->pos = 0;
    sb->len = size;
    return sb;
}

void knet_buffer_destroy(kbuffer_t* sb) {
    verify(sb);
    if (!sb) {
        return;
    }
    if (sb->ptr) {
        destroy(sb->ptr);
    }
    if (sb) {
        destroy(sb);
    }
}

uint32_t knet_buffer_put(kbuffer_t* sb, const char* temp, uint32_t size) {
    verify(sb);
    verify(temp);
    verify(size);
    if (!sb || !temp || !size) {
        return 0;
    }
    if (size > sb->len - sb->pos) {
        return 0;
    }
    memcpy(sb->ptr + sb->pos, temp, size);
    sb->pos += size;
    return size;
}

uint32_t knet_buffer_get_length(kbuffer_t* sb) {
    verify(sb);
    if (!sb) {
        return 0;
    }
    return sb->pos;
}

uint32_t knet_buffer_get_max_size(kbuffer_t* sb) {
    verify(sb);
    if (!sb) {
        return 0;
    }
    return sb->len;
}

int knet_buffer_enough(kbuffer_t* sb, uint32_t size) {
    verify(sb);
    if (!sb) {
        return 0;
    }
    return (sb->pos + size > sb->len);
}

char* knet_buffer_get_ptr(kbuffer_t* sb) {
    verify(sb);
    if (!sb) {
        return 0;
    }
    return sb->ptr;
}

void knet_buffer_adjust(kbuffer_t* sb, uint32_t gap) {
    verify(sb); /* gap可以为0 */
    if (!sb) {
        return;
    }
    sb->ptr += gap;
}

void knet_buffer_clear(kbuffer_t* sb) {
    verify(sb);
    if (!sb) {
        return;
    }
    if (!sb->ptr) {
        return;
    }
    sb->ptr = sb->ptr - sb->pos;
    sb->pos = 0;
}
