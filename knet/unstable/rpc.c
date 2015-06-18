#include "rpc.h"
#include "hash.h"
#include "channel_ref.h"
#include "stream.h"

/* TODO 原型开发中... */

/*************** 协议架构 **********************
 |   2字节   |  2字节 |  4字节  |    属性/结果   |
 \        /  \     /  \      / \             /
    长度        ID       类型        <=65535
 **********************************************
 ***************** 属性/结果 *******************
 |  key长度  |  key  |     value长度   |   value   |
  \        / \     /  \              / \         /
    1字节       内容        2字节            内容
*/

#if defined(_MSC_VER )
    #pragma pack(push)
    #pragma pack(1)
#else
    #pragma pack(1) 
#endif /* defined(_MSC_VER) */

typedef struct krpc_header_t {
    uint16_t length;       /* 包长度，包含本字段 */
    uint16_t rpcid;        /* 需要调用的方法ID/调用返回方法ID */
    uint8_t  type;         /* 类型（调用/返回) */
    uint8_t  padding__[3]; /* 3个字节填充，总长度为8字节 */
} krpc_header_t;

typedef struct krpc_attr_key_t {
    uint8_t length;        /* 长度 */
    char    key[1];        /* key内容 */
} krpc_attr_key_t;

typedef struct krpc_attr_value_t {
    uint16_t length;       /* 长度 */
    char     value[1];     /* value内容 */
} krpc_attr_value_t;

typedef struct krpc_object_header_t {
    uint8_t  type;          /* 类型 */
    uint16_t length;        /* 长度 */
    char     object[1];     /* 对象内容 */
} krpc_object_header_t;

#if defined(_MSC_VER )
    #pragma pack(pop)
#else
    #pragma pack()
#endif /* defined(_MSC_VER) */

typedef enum _krpc_call_type_e {
    krpc_call_type_result = 1,
    krpc_call_type_call = 2,
} krpc_call_type_e;

struct _krpc_t {
    hash_t* cb_table;
};

enum _krpc_type_e {
    krpc_type_i8 = 1,
    krpc_type_ui8 = 2,
    krpc_type_i16 = 4,
    krpc_type_ui16 = 8,
    krpc_type_i32 = 16,
    krpc_type_ui32 = 32,
    krpc_type_i64 = 64,
    krpc_type_ui64 = 128,
    krpc_type_f32 = 256,
    krpc_type_f64 = 512,
    krpc_type_number = 1024,
    krpc_type_string = 2048,
    krpc_type_vector = 4096,
};

struct _krpc_number_t {
    union {
        int8_t    i8;
        uint8_t   ui8;
        int16_t   i16;
        uint16_t  ui16;
        int32_t   i32;
        uint32_t  ui32;
        int64_t   i64;
        uint64_t  ui64;
        float32_t f32;
        float64_t f64;
    };
};

struct _krpc_string_t {
    uint16_t size;
    char*    str;
};

struct _krpc_vector_t {
    uint16_t        max_size;
    uint16_t        size;
    krpc_object_t** objects;
};

struct _krpc_object_t {
    krpc_type_e type;
    union {
        krpc_number_t number;
        krpc_string_t string;
        krpc_vector_t vector;
    };
};

struct _krpc_attribute_t {
    hash_t* attributes;
};

krpc_t* krpc_create() {
    krpc_t* frpc = create(krpc_t);
    assert(frpc);
    memset(frpc, 0, sizeof(krpc_t));
    frpc->cb_table = hash_create(0, 0);
    assert(frpc->cb_table);
    return frpc;
}

void krpc_destroy(krpc_t* frpc) {
    assert(frpc);
    hash_destroy(frpc->cb_table);
    destroy(frpc);
}

int krpc_add_cb(krpc_t* frpc, uint16_t rpcid, krpc_cb_t cb) {
    assert(frpc);
    assert(rpcid);
    assert(cb);
    if (hash_get(frpc->cb_table, rpcid)) {
        return error_rpc_dup_id;
    }
    return hash_add(frpc->cb_table, rpcid, cb);
}

int krpc_del_cb(krpc_t* frpc, uint16_t rpcid) {
    assert(frpc);
    assert(rpcid);
    return hash_delete(frpc->cb_table, rpcid);
}

krpc_cb_t krpc_get_cb(krpc_t* frpc, uint16_t rpcid) {
    assert(frpc);
    assert(rpcid);
    return (krpc_cb_t)hash_get(frpc->cb_table, rpcid);
}

int krpc_attribute_marshal(krpc_attribute_t* attribute, channel_ref_t* channel_ref) {
    assert(attribute);
    assert(channel_ref);
    return error_ok;
}

int krpc_attribute_unmarshal(krpc_attribute_t** attribute, channel_ref_t* channel_ref, int size) {
    assert(attribute);
    assert(channel_ref);
    assert(size);
    return error_ok;
}

uint16_t krpc_attribute_get_marshal_size(krpc_attribute_t* attribute) {
    assert(attribute);
    return 0;
}

int krpc_proc(krpc_t* frpc, channel_ref_t* channel_ref) {
    krpc_attribute_t* attribute = 0;
    krpc_cb_t         cb        = 0;
    int               bytes     = 0;
    int               error     = error_ok;
    krpc_header_t     header    = {0, 0};
    stream_t*         stream    = 0;
    assert(frpc);
    assert(channel_ref);
    stream = channel_ref_get_stream(channel_ref);
    bytes = stream_available(stream);
    if (bytes < sizeof(krpc_header_t)) { /* 没有足够的数据 */
        error = error_rpc_next;
        goto success_return;
    }
    stream_copy(stream, (char*)&header, sizeof(krpc_header_t));
    if (header.length > bytes) { /* 没有足够的数据 */
        error = error_rpc_next;
        goto success_return;
    }
    error = stream_eat(stream, sizeof(krpc_header_t));
    if (error != error_ok) {
        goto error_return;
    }
    if (header.length != bytes) { /* 有参数的调用 */
        /* unmarshal调用参数 */
        error = krpc_attribute_unmarshal(&attribute, channel_ref, header.length);
        if (error != error_ok) {
            goto error_return;
        }
    }
    /* 查找回调 */
    cb = (krpc_cb_t)hash_get(frpc->cb_table, header.rpcid);
    if (!cb) {
        error = error_rpc_unknown_id;
        goto error_return;
    }
    error = cb(attribute);
    if (error != rpc_ok) {
        if (error == rpc_error) {
            /* 发生错误，但不关闭 */
            error = error_rpc_cb_fail;
            goto success_return;
        } else if (error == rpc_error_close) {
            goto error_return;
        }
    }
success_return:
    return error;
error_return:
    channel_ref_close(channel_ref);
    return error;
}

int krpc_call(krpc_t* frpc, channel_ref_t* channel_ref, uint16_t rpcid, krpc_attribute_t* attribute) {
    krpc_header_t header = {0, 0};
    int           error  = error_ok;
    stream_t*     stream = 0;
    assert(frpc);
    assert(channel_ref);
    assert(rpcid);
    assert(attribute);
    stream = channel_ref_get_stream(channel_ref);
    header.length = krpc_attribute_get_marshal_size(attribute);
    header.rpcid  = rpcid;
    header.type   = krpc_call_type_call;
    error = stream_push(stream, (char*)&header, sizeof(krpc_header_t));
    if (error != error_ok) {
        /* 错误，关闭 */
        channel_ref_close(channel_ref);
    } else {
        error = krpc_attribute_marshal(attribute, channel_ref);
        if (error != error_ok) {
            /* 错误，关闭 */
            channel_ref_close(channel_ref);
        }
    }
    return error;
}

krpc_attribute_t* krpc_attribute_create() {
    krpc_attribute_t* attribute = create(krpc_attribute_t);
    assert(attribute);
    memset(attribute, 0, sizeof(krpc_attribute_t));
    /* TODO 清理函数 */
    attribute->attributes = hash_create(8, 0);
    assert(attribute->attributes);
    return attribute;
}

void krpc_attribute_destroy(krpc_attribute_t* attribute) {
    assert(attribute);
    hash_destroy(attribute->attributes);
    destroy(attribute);
}

int krpc_attribute_append(krpc_attribute_t* attribute, uint32_t key, krpc_object_t* o) {
    assert(attribute);
    assert(o);
    /* TODO 是否支持覆盖？ */
    return hash_add(attribute->attributes, key, o);
}

int krpc_attribute_append_string_key(krpc_attribute_t* attribute, const char* key, krpc_object_t* o) {
    assert(attribute);
    assert(key);
    assert(o);
    return hash_add_string_key(attribute->attributes, key, o);
}

krpc_object_t* krpc_attribute_get(krpc_attribute_t* attribute, uint32_t key) {
    assert(attribute);
    return (krpc_object_t*)hash_get(attribute->attributes, key);
}

krpc_object_t* krpc_attribute_get_string_key(krpc_attribute_t* attribute, const char* key) {
    assert(attribute);
    assert(key);
    return (krpc_object_t*)hash_get_string_key(attribute->attributes, key);
}

krpc_object_t* krpc_create_object() {
    krpc_object_t* o = create(krpc_object_t);
    assert(o);
    memset(o, 0, sizeof(krpc_object_t));
    return o;
}

void krpc_destroy_object(krpc_object_t* o) {
    assert(o);
    /* TODO 销毁 */
    destroy(o);
}

int krpc_object_check_type(krpc_object_t* o, krpc_type_e type) {
    if (!o->type) {
        return 1;
    }
    return (o->type & type);
}

krpc_number_t* krpc_object_cast_number(krpc_object_t* o) {
    if (o->type & krpc_type_number) {
        return &o->number;
    }
    return 0;
}

krpc_string_t* krpc_object_cast_string(krpc_object_t* o) {
    if (krpc_type_string & o->type) {
        return &o->string;
    }
    return 0;
}

krpc_vector_t* krpc_object_cast_vector(krpc_object_t* o) {
    if (krpc_type_vector & o->type) {
        return &o->vector;
    }
    return 0;
}

void krpc_number_set_i8(krpc_object_t* o, int8_t i8) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_i8;
    o->number.i8 = i8;
}

int8_t krpc_number_get_i8(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i8;
}

void krpc_number_set_i16(krpc_object_t* o, int16_t i16) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_i16;
    o->number.i16 = i16;
}

int16_t krpc_number_get_i16(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i16;
}

void krpc_number_set_i32(krpc_object_t* o, int32_t i32) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_i32;
    o->number.i32 = i32;
}

int32_t krpc_number_get_i32(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i32;
}

void krpc_number_set_i64(krpc_object_t* o, int32_t i64) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_i64;
    o->number.i64 = i64;
}

int64_t krpc_number_get_i64(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.i64;
}

void krpc_number_set_ui8(krpc_object_t* o, uint8_t ui8) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_ui8;
    o->number.ui8 = ui8;
}

uint8_t krpc_number_get_ui8(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui8;
}

void krpc_number_set_ui16(krpc_object_t* o, uint16_t ui16) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_ui16;
    o->number.ui16 = ui16;
}

uint16_t krpc_number_get_ui16(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui16;
}

void krpc_number_set_ui32(krpc_object_t* o, uint32_t ui32) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_ui32;
    o->number.ui32 = ui32;
}

uint32_t krpc_number_get_ui32(krpc_object_t* o) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    return o->number.ui32;
}

void krpc_number_set_ui64(krpc_object_t* o, int32_t ui64) {
    if (!krpc_object_check_type(o, krpc_type_number)) {
        assert(0);
    }
    o->type |= krpc_type_number | krpc_type_ui64;
    o->number.ui64 = ui64;
}

uint64_t krpc_number_get_ui64(krpc_object_t* o) {
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
    if (o->string.str) {
        if (o->string.size < size) {
            o->string.str = rcreate_raw(o->string.str, size);
        }
    } else {
        o->string.str = create_type(char, size);
    }
    assert(o->string.str);
    o->string.size = size;
    memcpy(o->string.str, s, size);
    o->string.str[size] = 0;
    o->type |= krpc_type_string;
}

void krpc_string_set_s(krpc_object_t* o, const char* s, uint32_t size) {
    assert(o);
    assert(s);
    if (!krpc_object_check_type(o, krpc_type_string)) {
        assert(0);
    }
    if (s[size - 1]) {
        size++;
    }
    if (o->string.str) {
        if (o->string.size < size) {
            destroy(o->string.str);
            o->string.str = create_type(char, size);
        }
    } else {
        o->string.str = create_type(char, size);
    }
    assert(o->string.str);
    o->string.size = (uint16_t)size;
    memcpy(o->string.str, s, size);
    o->string.str[size] = 0;
    o->type |= krpc_type_string;
}

const char* krpc_string_get(krpc_object_t* o) {
    assert(o);
    return o->string.str;
}

uint32_t krpc_string_get_size(krpc_object_t* o) {
    assert(o);
    return o->string.size;
}

int krpc_vector_push_back(krpc_object_t* v, krpc_object_t* o) {
    assert(v);
    assert(o);
    if (v->vector.objects) {
        v->vector.max_size = 8;
        v->vector.objects = (krpc_object_t**)create_type(krpc_object_t,
            sizeof(krpc_object_t*) * v->vector.max_size);
        assert(v->vector.objects);
    }
    if (v->vector.size >= v->vector.max_size) {
        v->vector.max_size += 8;
        v->vector.objects = (krpc_object_t**)rcreate_type(krpc_object_t,
            v->vector.objects, sizeof(krpc_object_t*) * v->vector.max_size);
    }
    v->vector.objects[v->vector.size] = o;
    v->vector.size++;
    o->type |= krpc_type_vector;
    return error_ok;
}

uint32_t krpc_vector_get_size(krpc_object_t* v) {
    assert(v);
    return v->vector.size;
}

krpc_object_t* krpc_vector_get(krpc_object_t* v, int index) {
    assert(v);
    if (index >= v->vector.size) {
        return 0;
    }
    return v->vector.objects[index];
}

int krpc_vector_set(krpc_object_t* v, krpc_object_t* o, int index) {
    assert(v);
    assert(o);
    if (index >= v->vector.size) {
        return error_rpc_vector_out_of_bound;
    }
    if (v->vector.objects[index]) {
        krpc_destroy_object(v->vector.objects[index]);
    }
    v->vector.objects[index] = o;
    o->type |= krpc_type_vector;
    return error_ok;
}

uint16_t krpc_object_marshal(krpc_object_t* o, char* buffer, uint16_t size) {
    uint16_t bytes  = 0;
    uint16_t retval = 0;
    assert(o);
    assert(buffer);
    assert(size);
    if (krpc_object_check_type(o, krpc_type_number)) {
        retval = krpc_number_marshal(o, buffer, size);
        if (!retval) {
            return 0;
        }
    } else if (krpc_object_check_type(o, krpc_type_string)) {
        retval = krpc_string_marshal(o, buffer, size);
        if (!retval) {
            return 0;
        }
    } else if (krpc_object_check_type(o, krpc_type_vector)) {
        retval = krpc_vector_marshal(o, buffer, size);
        if (!retval) {
            return 0;
        }
    }
    bytes += retval;
    return bytes;
}

uint16_t krpc_number_get_marshal_size(krpc_object_t* o) {
    uint16_t bytes = sizeof(krpc_object_header_t) - 1;
    assert(o);
    if (krpc_object_check_type(o, krpc_type_i8)) {
        bytes += sizeof(int8_t);
    } else if (krpc_object_check_type(o, krpc_type_i16)) {
        bytes += sizeof(int16_t);
    } else if (krpc_object_check_type(o, krpc_type_i32)) {
        bytes += sizeof(int32_t);
    } else if (krpc_object_check_type(o, krpc_type_i64)) {
        bytes += sizeof(int64_t);
    } else if (krpc_object_check_type(o, krpc_type_ui8)) {
        bytes += sizeof(uint8_t);
    } else if (krpc_object_check_type(o, krpc_type_ui16)) {
        bytes += sizeof(uint16_t);
    } else if (krpc_object_check_type(o, krpc_type_ui32)) {
        bytes += sizeof(uint32_t);
    } else if (krpc_object_check_type(o, krpc_type_ui64)) {
        bytes += sizeof(uint64_t);
    } else if (krpc_object_check_type(o, krpc_type_f32)) {
        bytes += sizeof(float32_t);
    } else if (krpc_object_check_type(o, krpc_type_f64)) {
        bytes += sizeof(float64_t);
    } else {
        return 0;
    }
    return bytes;
}

uint16_t krpc_number_marshal(krpc_object_t* o, char* buffer, uint16_t size) {
    krpc_object_header_t* header = 0;
    assert(o);
    assert(buffer);
    assert(size);
    header = (krpc_object_header_t*)buffer;
    header->type = o->type;
    header->length = sizeof(krpc_object_header_t) - 1;
    if (krpc_object_check_type(o, krpc_type_i8)) {
        header->length += sizeof(int8_t);
        memcpy(header->object, &o->number.i8, sizeof(int8_t));
    } else if (krpc_object_check_type(o, krpc_type_i16)) {
        header->length += sizeof(int16_t);
        memcpy(header->object, &o->number.i16, sizeof(int16_t));
    } else if (krpc_object_check_type(o, krpc_type_i32)) {
        header->length += sizeof(int32_t);
        memcpy(header->object, &o->number.i32, sizeof(int32_t));
    } else if (krpc_object_check_type(o, krpc_type_i64)) {
        header->length += sizeof(int64_t);
        memcpy(header->object, &o->number.i64, sizeof(int64_t));
    } else if (krpc_object_check_type(o, krpc_type_ui8)) {
        header->length += sizeof(uint8_t);
        memcpy(header->object, &o->number.ui8, sizeof(uint8_t));
    } else if (krpc_object_check_type(o, krpc_type_ui16)) {
        header->length += sizeof(uint16_t);
        memcpy(header->object, &o->number.ui16, sizeof(uint16_t));
    } else if (krpc_object_check_type(o, krpc_type_ui32)) {
        header->length += sizeof(uint32_t);
        memcpy(header->object, &o->number.ui32, sizeof(uint32_t));
    } else if (krpc_object_check_type(o, krpc_type_ui64)) {
        header->length += sizeof(uint64_t);
        memcpy(header->object, &o->number.ui64, sizeof(uint64_t));
    } else if (krpc_object_check_type(o, krpc_type_f32)) {
        header->length += sizeof(float32_t);
        memcpy(header->object, &o->number.f32, sizeof(float32_t));
    } else if (krpc_object_check_type(o, krpc_type_f64)) {
        header->length += sizeof(float64_t);
        memcpy(header->object, &o->number.f64, sizeof(float64_t));
    } else {
        return 0;
    }
    return header->length;
}

uint16_t krpc_string_get_marshal_size(krpc_object_t* o) {
    assert(o);
    return sizeof(krpc_object_header_t) + o->string.size - 1;
}

uint16_t krpc_string_marshal(krpc_object_t* o, char* buffer, uint16_t size) {
    krpc_object_header_t* header = 0;
    assert(0);
    assert(buffer);
    assert(size);
    header = (krpc_object_header_t*)buffer;
    header->type = o->type;
    header->length = sizeof(krpc_object_header_t) - 1;
    header->length += o->string.size;
    memcpy(header->object, o->string.str, o->string.size);
    return header->length;
}

uint16_t krpc_vector_get_marshal_size(krpc_object_t* o) {
    uint16_t i = 0;
    uint16_t bytes = sizeof(krpc_object_header_t) - 1;
    krpc_object_t* e = 0;
    assert(0);
    for (; i < o->vector.size; i++) {
        e = o->vector.objects[i];
        if (krpc_object_check_type(e, krpc_type_number)) {
            bytes += krpc_number_get_marshal_size(e);
        } else if (krpc_object_check_type(e, krpc_type_string)) {
            bytes += krpc_string_get_marshal_size(e);
        } else if (krpc_object_check_type(e, krpc_type_vector)) {
            bytes += krpc_vector_get_marshal_size(e);
        } else {
            return 0;
        }
    }
    return bytes;
}

uint16_t krpc_vector_marshal(krpc_object_t* o, char* buffer, uint16_t size) {
    krpc_object_header_t* header = 0;
    uint16_t i = 0;
    uint16_t bytes = sizeof(krpc_object_header_t) - 1;
    krpc_object_t* e = 0;
    assert(0);
    assert(buffer);
    assert(size);
    header = (krpc_object_header_t*)buffer;
    for (; i < o->vector.size; i++) {
        e = o->vector.objects[i];
        if (krpc_object_check_type(e, krpc_type_number)) {
            bytes += krpc_number_marshal(e, buffer + bytes, size - bytes);
        } else if (krpc_object_check_type(e, krpc_type_string)) {
            bytes += krpc_string_marshal(e, buffer + bytes, size - bytes);
        } else if (krpc_object_check_type(e, krpc_type_vector)) {
            bytes += krpc_vector_marshal(e, buffer + bytes, size - bytes);
        } else {
            return 0;
        }
    }
    return bytes;
}

int krpc_object_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o) {
    krpc_object_header_t header;
    stream_t* stream = 0;
    uint32_t bytes = 0;
    assert(channel_ref);
    assert(o);
    memset(&header, 0, sizeof(header));
    stream = channel_ref_get_stream(channel_ref);
    assert(stream);
    bytes = stream_available(stream);
    if (bytes < sizeof(krpc_object_header_t)) {
        stream_copy(stream, (char*)&header, sizeof(krpc_object_header_t));
        if (header.length < bytes) {
            return 0;
        }
    }
    if (header.type & krpc_type_number) {
        return krpc_number_unmarshal(channel_ref, o);
    } else if (header.type & krpc_type_string) {
        return krpc_string_unmarshal(channel_ref, o);
    } else if (header.type & krpc_type_vector) {
        return krpc_vector_unmarshal(channel_ref, o);
    }
    return 0;
}

int krpc_number_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o) {
    assert(channel_ref);
    assert(o);
    return 0;
}

int krpc_string_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o) {
    assert(channel_ref);
    assert(o);
    return 0;
}

int krpc_vector_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o) {
    assert(channel_ref);
    assert(o);
    return 0;
}
