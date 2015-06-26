//
// Generated code, DO NOT modify
//

#ifndef _krpc_rpc_sample_h_
#define _krpc_rpc_sample_h_

#include <cstdint>
#include <string>
#include <vector>
#include "knet.h"

struct my_message_t;
struct my_object_t;

struct my_message_t {
    std::string str;
    my_message_t();
    my_message_t(const my_message_t& rht);
    const my_message_t& operator=(const my_message_t& rht);
    void print(std::stringstream& ss, std::string white = "");
};

struct my_object_t {
    int8_t n_i8;
    int16_t n_i16;
    int32_t n_i32;
    int64_t n_i64;
    uint8_t n_ui8;
    uint16_t n_ui16;
    uint32_t n_ui32;
    uint64_t n_ui64;
    float n_f32;
    double n_f64;
    std::vector<my_message_t> message;
    std::vector<int32_t> test_array;
    my_object_t();
    my_object_t(const my_object_t& rht);
    const my_object_t& operator=(const my_object_t& rht);
    void print(std::stringstream& ss, std::string white = "");
};

krpc_object_t* marshal(my_message_t& o);
bool unmarshal(krpc_object_t* v, my_message_t& o);
krpc_object_t* marshal(my_object_t& o);
bool unmarshal(krpc_object_t* v, my_object_t& o);
krpc_object_t* my_rpc_func_proxy(my_object_t& my_obj, std::vector<my_object_t>& obj_array, const std::string& test_str, std::vector<float>& array_f32);
int my_rpc_func_stub(krpc_object_t* o);
int my_rpc_func(my_object_t& my_obj, std::vector<my_object_t>& obj_array, const std::string& test_str, std::vector<float>& array_f32);

class rpc_sample_t {
public:
    ~rpc_sample_t();
    static rpc_sample_t* instance();
    static void finalize();
    int rpc_proc(stream_t* stream);
    int my_rpc_func(stream_t* stream, my_object_t& my_obj, std::vector<my_object_t>& obj_array, const std::string& test_str, std::vector<float>& array_f32);
private:
    rpc_sample_t();
    rpc_sample_t(const rpc_sample_t&);
    static rpc_sample_t* _instance;
    krpc_t* _rpc;
};

#endif // _krpc_rpc_sample_h_
