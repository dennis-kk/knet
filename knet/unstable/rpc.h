#ifndef RPC_H
#define RPC_H

#include "config.h"

/* TODO 原型开发中... */

krpc_t* krpc_create();
void krpc_destroy(krpc_t* frpc);
int krpc_add_cb(krpc_t* frpc, uint16_t rpcid, krpc_cb_t cb);
int krpc_del_cb(krpc_t* frpc, uint16_t rpcid);
krpc_cb_t krpc_get_cb(krpc_t* frpc, uint16_t rpcid);
int krpc_proc(krpc_t* frpc, channel_ref_t* channel_ref);
int krpc_call(krpc_t* frpc, channel_ref_t* channel_ref, uint16_t rpcid, krpc_attribute_t* attribute);

krpc_attribute_t* krpc_attribute_create();
void krpc_attribute_destroy(krpc_attribute_t* attribute);
int krpc_attribute_append(krpc_attribute_t* attribute, uint32_t key, krpc_object_t* o);
int krpc_attribute_append_string_key(krpc_attribute_t* attribute, const char* key, krpc_object_t* o);
krpc_object_t* krpc_attribute_get(krpc_attribute_t* attribute, uint32_t key);
krpc_object_t* krpc_attribute_get_string_key(krpc_attribute_t* attribute, const char* key);
int krpc_attribute_marshal(krpc_attribute_t* attribute, channel_ref_t* channel_ref);
int krpc_attribute_unmarshal(krpc_attribute_t** attribute, channel_ref_t* channel_ref, int size);
uint16_t krpc_attribute_get_marshal_size(krpc_attribute_t* attribute);

krpc_object_t* krpc_create_object();
void krpc_destroy_object(krpc_object_t* o);
int krpc_object_check_type(krpc_object_t* o, krpc_type_e type);
krpc_number_t* krpc_object_cast_number(krpc_object_t* o);
krpc_string_t* krpc_object_cast_string(krpc_object_t* o);
krpc_vector_t* krpc_object_cast_vector(krpc_object_t* o);
uint16_t krpc_object_get_marshal_size(krpc_object_t* o);
uint16_t krpc_object_marshal(krpc_object_t* o, char* buffer, uint16_t size);
int krpc_object_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o);

void krpc_number_set_i8(krpc_object_t* o, int8_t i8);
int8_t krpc_number_get_i8(krpc_object_t* o);
void krpc_number_set_i16(krpc_object_t* o, int16_t i16);
int16_t krpc_number_get_i16(krpc_object_t* o);
void krpc_number_set_i32(krpc_object_t* o, int32_t i32);
int32_t krpc_number_get_i32(krpc_object_t* o);
void krpc_number_set_i64(krpc_object_t* o, int32_t i64);
int64_t krpc_number_get_i64(krpc_object_t* o);
void krpc_number_set_ui8(krpc_object_t* o, uint8_t ui8);
uint8_t krpc_number_get_ui8(krpc_object_t* o);
void krpc_number_set_ui16(krpc_object_t* o, uint16_t ui16);
uint16_t krpc_number_get_ui16(krpc_object_t* o);
void krpc_number_set_ui32(krpc_object_t* o, uint32_t ui32);
uint32_t krpc_number_get_ui32(krpc_object_t* o);
void krpc_number_set_ui64(krpc_object_t* o, int32_t ui64);
uint64_t krpc_number_get_ui64(krpc_object_t* o);
uint16_t krpc_number_get_marshal_size(krpc_object_t* o);
uint16_t krpc_number_marshal(krpc_object_t* o, char* buffer, uint16_t size);
int krpc_number_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o, uint8_t type, uint16_t size);

void krpc_string_set(krpc_object_t* o, const char* s);
void krpc_string_set_s(krpc_object_t* o, const char* s, uint32_t size);
const char* krpc_string_get(krpc_object_t* o);

void krpc_string_set_size(krpc_object_t* o, uint16_t size);
uint16_t krpc_string_get_size(krpc_object_t* o);
uint16_t krpc_string_get_marshal_size(krpc_object_t* o);
uint16_t krpc_string_marshal(krpc_object_t* o, char* buffer, uint16_t size);
int krpc_string_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o, uint16_t size);

int krpc_vector_push_back(krpc_object_t* v, krpc_object_t* o);
uint32_t krpc_vector_get_size(krpc_object_t* v);
krpc_object_t* krpc_vector_get(krpc_object_t* v, int index);
int krpc_vector_set(krpc_object_t* v, krpc_object_t* o, int index);
uint16_t krpc_vector_get_marshal_size(krpc_object_t* o);
uint16_t krpc_vector_marshal(krpc_object_t* o, char* buffer, uint16_t size);
int krpc_vector_unmarshal(channel_ref_t* channel_ref, krpc_object_t** o, uint16_t size);

#endif /* RPC_H */
