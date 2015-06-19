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

/* TODO 待测试 */

#include "rpc_object.h"
#include "stream.h"

#if defined(_MSC_VER )
    #pragma pack(push)
    #pragma pack(1)
#else
    #pragma pack(1) 
#endif /* defined(_MSC_VER) */

typedef struct krpc_object_header_t {
    uint16_t  type;   /* 类型 */
    uint16_t  length; /* 长度，包含本结构体 */
} krpc_object_header_t;

#if defined(_MSC_VER )
    #pragma pack(pop)
#else
    #pragma pack()
#endif /* defined(_MSC_VER) */

enum _krpc_type_e {
    krpc_type_i8     = 1,    /* 有符号8位 */
    krpc_type_ui8    = 2,    /* 无符号8位 */
    krpc_type_i16    = 4,    /* 有符号16位 */
    krpc_type_ui16   = 8,    /* 无符号16位 */
    krpc_type_i32    = 16,   /* 有符号32位 */
    krpc_type_ui32   = 32,   /* 无符号32位 */
    krpc_type_i64    = 64,   /* 有符号64位 */
    krpc_type_ui64   = 128,  /* 无符号64位 */
    krpc_type_f32    = 256,  /* 32位浮点 */
    krpc_type_f64    = 512,  /* 64位浮点 */
    krpc_type_number = 1024, /* 数字 */
    krpc_type_string = 2048, /* 字符串 */
    krpc_type_vector = 4096, /* 数组 */
};

struct _krpc_number_t {
    union {
        int8_t    i8;   /* 有符号8位 */
        uint8_t   ui8;  /* 无符号8位 */
        int16_t   i16;  /* 有符号16位 */
        uint16_t  ui16; /* 无符号16位 */
        int32_t   i32;  /* 有符号32位 */
        uint32_t  ui32; /* 无符号32位 */
        int64_t   i64;  /* 有符号64位 */
        uint64_t  ui64; /* 无符号64位 */
        float32_t f32;  /* 32位浮点 */
        float64_t f64;  /* 64位浮点 */
    };
};

struct _krpc_string_t {
    uint16_t size; /* 字符串长度 */
    char*    str;  /* 字符串指针 */
};

struct _krpc_vector_t {
    uint16_t        max_size; /* 最大元素个数 */
    uint16_t        size;     /* 当前元素个数 */
    krpc_object_t** objects;  /* 对象指针数组 */
};

struct _krpc_object_t {
    uint16_t type;
    union {
        krpc_number_t number; /* 数字 */
        krpc_string_t string; /* 字符串 */
        krpc_vector_t vector; /* 数组 */
    };
};

krpc_object_t* krpc_object_create() {
    krpc_object_t* o = create(krpc_object_t);
    assert(o);
    memset(o, 0, sizeof(krpc_object_t));
    return o;
}

void krpc_object_destroy(krpc_object_t* o) {
    int i = 0;
    assert(o);
    if (o->type & krpc_type_string) {
        /* 字符串 */
        if (o->string.str) {
            destroy(o->string.str);
        }
    } else if (o->type & krpc_type_vector) {
        /* 数组 */
        for (; i < o->vector.size; i++) {
            /* 递归销毁 */
            krpc_object_destroy(o->vector.objects[i]);
        }
    } else {
        /* 数字 */
    }
    destroy(o);
}

int krpc_object_check_type(krpc_object_t* o, krpc_type_e type) {
    assert(o);
    if (!o->type) {
        return 1;
    }
    return (o->type & type);
}

uint16_t krpc_number_get_marshal_size(krpc_object_t* o) {
    if (o->type & krpc_type_i8) {
        return sizeof(int8_t);
    } else if (o->type & krpc_type_i16) {
        return sizeof(int16_t);
    } else if (o->type & krpc_type_i32) {
        return  sizeof(int32_t);
    } else if (o->type & krpc_type_i64) {
        return sizeof(int64_t);
    } else if (o->type & krpc_type_ui8) {
        return sizeof(uint8_t);
    } else if (o->type & krpc_type_ui16) {
        return sizeof(uint16_t);
    } else if (o->type & krpc_type_ui32) {
        return sizeof(uint32_t);
    } else if (o->type & krpc_type_ui64) {
        return sizeof(uint64_t);
    } else if (o->type & krpc_type_f32) {
        return sizeof(float32_t);
    } else if (o->type & krpc_type_f64) {
        return sizeof(float64_t);
    }
    return 0;
}

uint16_t krpc_object_get_marshal_size(krpc_object_t* o) {
    uint16_t size = sizeof(krpc_object_header_t);
    uint16_t i    = 0;
    if (o->type & krpc_type_string) {
        /* 字符串 */
        size += o->string.size;
    } else if (o->type & krpc_type_vector) {
        /* 数组 */
        for (; i < o->vector.size; i++) {
            size += krpc_object_get_marshal_size(o->vector.objects[i]);
        }
    } else if (o->type & krpc_type_number) {
        /* 数字 */
        size += krpc_number_get_marshal_size(o);
    } else {
        return 0;
    }
    return size;
}

int krpc_object_marshal(krpc_object_t* o, stream_t* stream, uint16_t* bytes) {
    uint16_t size = 0;
    int      i    = 0;
    krpc_object_header_t header;
    assert(o);
    assert(stream);
    memset(&header, 0, sizeof(krpc_object_header_t));
    header.type   = o->type;
    header.length += krpc_object_get_marshal_size(o);
    if (error_ok != stream_push(stream, &header, sizeof(krpc_object_header_t))) {
        return error_rpc_marshal_fail;
    }
    if (o->type & krpc_type_string) {
        /* 字符串 */
        if (error_ok != stream_push(stream, o->string.str, o->string.size)) {
            return error_rpc_marshal_fail;
        }
    } else if (o->type & krpc_type_vector) {
        /* 数组 */
        for (; i < o->vector.size; i++) {
            /* 递归调用 */
            if (error_ok != krpc_object_marshal(o->vector.objects[i], stream, &size)) {
                return error_rpc_marshal_fail;
            }
        }
    } else if (o->type & krpc_type_number) {
        /* 数字 */
        if (error_ok != stream_push(stream, &o->number, krpc_number_get_marshal_size(o))) {
            return error_rpc_marshal_fail;
        }
    } else {
        return error_rpc_marshal_fail;
    }
    if (bytes) {
        *bytes = header.length;
    }
    return error_ok;
}

int krpc_object_unmarshal(stream_t* stream, krpc_object_t** o, uint16_t* bytes) {
    uint16_t       length    = 0; /* 临时变量 */
    int            available = 0; /* 可读字节数 */
    uint16_t       consume   = 0; /* 单次unmarshal消耗字节数 */
    krpc_object_t* vo        = 0; /* 数组内对象指针 */
    krpc_object_header_t header;  /* 对象协议头 */
    assert(stream);
    assert(o);
    assert(bytes);
    available = stream_available(stream);
    if (available < sizeof(krpc_object_header_t)) {
        /* 字节数不够 */
        return error_rpc_not_enough_bytes;
    }
    if (error_ok != stream_copy(stream, &header, sizeof(header))) {
        return error_rpc_unmarshal_fail;
    }
    if (header.length > (uint16_t)available) {
        /* 字节数不够 */
        return error_rpc_not_enough_bytes;
    }
    if (error_ok != stream_eat(stream, sizeof(krpc_object_header_t))) {
        return error_rpc_unmarshal_fail;
    }
    /* 建立一个对象 */
    *o = krpc_object_create();
    assert(*o);
    if (header.type & krpc_type_number) {
        /* 数字 */
        if (error_ok != stream_pop(stream, &(*o)->number, header.length - sizeof(krpc_object_header_t))) {
            goto error_return;
        }
    } else if (header.type & krpc_type_string) {
        /* 字符串 */
        if (error_ok != krpc_string_set_size(*o, header.length - sizeof(krpc_object_header_t))) {
            goto error_return;
        }
        if (error_ok != stream_pop(stream, (*o)->string.str, header.length - sizeof(krpc_object_header_t))) {
            goto error_return;
        }
        (*o)->string.size = header.length - sizeof(krpc_object_header_t);
    } else if (header.type & krpc_type_vector) {
        /* 数组 */
        length = header.length - sizeof(krpc_object_header_t);
        for (; length;) {
            /* 递归调用 */
            if (error_ok != krpc_object_unmarshal(stream, &vo, &consume)) {
                goto error_return;
            }
            if (error_ok != krpc_vector_push_back(*o, vo)) {
                goto error_return;
            }
            length -= consume;
        }
    } else {
        /* 未知类型 */
        goto error_return;
    }
    (*o)->type = (krpc_type_e)header.type; /* 类型 */
    *bytes     = header.length;            /* 消耗字节数 */
    return error_ok;
error_return:
    /* 错误处理 */
    if (*o) {
        krpc_object_destroy(*o);
        *o = 0;
    }
    /* 告诉调用者失败，外部应关闭本连接 */
    return error_rpc_unmarshal_fail;
}

void krpc_number_set_i8(krpc_object_t* o, int8_t i8) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_i8;
    o->number.i8 = i8;
}

int8_t krpc_number_get_i8(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i8;
}

void krpc_number_set_i16(krpc_object_t* o, int16_t i16) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_i16;
    o->number.i16 = i16;
}

int16_t krpc_number_get_i16(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i16;
}

void krpc_number_set_i32(krpc_object_t* o, int32_t i32) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_i32;
    o->number.i32 = i32;
}

int32_t krpc_number_get_i32(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i32;
}

void krpc_number_set_i64(krpc_object_t* o, int64_t i64) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_i64;
    o->number.i64 = i64;
}

int64_t krpc_number_get_i64(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i64;
}

void krpc_number_set_ui8(krpc_object_t* o, uint8_t ui8) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_ui8;
    o->number.ui8 = ui8;
}

uint8_t krpc_number_get_ui8(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui8;
}

void krpc_number_set_ui16(krpc_object_t* o, uint16_t ui16) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_ui16;
    o->number.ui16 = ui16;
}

uint16_t krpc_number_get_ui16(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui16;
}

void krpc_number_set_ui32(krpc_object_t* o, uint32_t ui32) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_ui32;
    o->number.ui32 = ui32;
}

uint32_t krpc_number_get_ui32(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui32;
}

void krpc_number_set_ui64(krpc_object_t* o, int32_t ui64) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type = krpc_type_number | krpc_type_ui64;
    o->number.ui64 = ui64;
}

uint64_t krpc_number_get_ui64(krpc_object_t* o) {
    assert(o);
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui64;
}

void krpc_string_set(krpc_object_t* o, const char* s) {
    uint16_t size = 0;
    assert(o);
    assert(s);
    if (!krpc_object_check_type(o, krpc_type_string)) {
        assert(0);
    }
    size = (uint16_t)strlen(s) + 1;
    if (!o->string.str) {
        o->string.str = create_type(char, size);
    }
    assert(o->string.str);
    o->string.size = size;
    memcpy(o->string.str, s, size);
    o->string.str[size - 1] = 0;
    o->type = krpc_type_string;
}

void krpc_string_set_s(krpc_object_t* o, const char* s, uint16_t size) {
    assert(o);
    assert(s);
    if (!krpc_object_check_type(o, krpc_type_string)) {
        assert(0);
    }
    if (!o->string.str) {
        o->string.str = create_type(char, size);
    }
    assert(o->string.str);
    o->string.size = size;
    memcpy(o->string.str, s, size);
    o->string.str[size - 1] = 0;
    o->type = krpc_type_string;
}

int krpc_string_set_size(krpc_object_t* o, uint16_t size) {
    assert(o);
    assert(size);
    if (!krpc_object_check_type(o, krpc_type_string)) {
        assert(0);
    }
    assert(!o->string.str);
    o->string.str  = create_type(char, size);
    o->string.size = size;
    o->type = krpc_type_string;
    return error_ok;
}

const char* krpc_string_get(krpc_object_t* o) {
    assert(o);
    if (!(o->type & krpc_type_string)) {
        assert(0);
    }
    return o->string.str;
}

uint16_t krpc_string_get_size(krpc_object_t* o) {
    assert(o);
    if (!(o->type & krpc_type_string)) {
        assert(0);
    }
    return o->string.size;
}

void krpc_vector_enlarge(krpc_object_t* o) {
    static const uint16_t DEFAULT_SIZE = 8;
    if (!o->vector.objects) { /* 第一次建立 */
        o->vector.max_size = DEFAULT_SIZE;
        o->vector.objects = (krpc_object_t**)create_type(krpc_object_t,
            sizeof(krpc_object_t*) * o->vector.max_size);
        assert(o->vector.objects);
        memset(o->vector.objects, 0, sizeof(krpc_object_t*) * o->vector.max_size);
    }
    if (o->vector.size >= o->vector.max_size) { /* 扩容 */
        o->vector.max_size += DEFAULT_SIZE;
        o->vector.objects = (krpc_object_t**)rcreate_type(krpc_object_t,
            o->vector.objects, sizeof(krpc_object_t*) * o->vector.max_size);
        assert(o->vector.objects);
        memset(o->vector.objects + (o->vector.max_size - DEFAULT_SIZE), 0, sizeof(krpc_object_t*) * DEFAULT_SIZE);
    }
}

int krpc_vector_push_back(krpc_object_t* v, krpc_object_t* o) {
    assert(v);
    assert(o);
    if (!krpc_object_check_type(v, krpc_type_vector)) {
        assert(0);
    }
    krpc_vector_enlarge(v);
    v->vector.objects[v->vector.size] = o;
    v->vector.size++;
    v->type = krpc_type_vector;
    return error_ok;
}

uint32_t krpc_vector_get_size(krpc_object_t* v) {
    assert(v);
    if (!(v->type & krpc_type_vector)) {
        assert(0);
    }
    return v->vector.size;
}

krpc_object_t* krpc_vector_get(krpc_object_t* v, int index) {
    assert(v);
    if (!(v->type & krpc_type_vector)) {
        assert(0);
    }
    if (index >= v->vector.size) {
        return 0;
    }
    return v->vector.objects[index];
}

int krpc_vector_set(krpc_object_t* v, krpc_object_t* o, int index) {
    assert(v);
    assert(o);
    if (!krpc_object_check_type(v, krpc_type_vector)) {
        assert(0);
    }
    if (index >= v->vector.size) {
        return error_rpc_vector_out_of_bound;
    }
    if (v->vector.objects[index]) {
        krpc_object_destroy(v->vector.objects[index]);
    }
    v->vector.objects[index] = o;
    v->type = krpc_type_vector;
    return error_ok;
}
