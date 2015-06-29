//
// KRPC - Generated code, DO NOT modify
//

#include <sstream>
#include "rpc_sample.h"

template<typename T>
void push_back_all(std::vector<T>& v, typename std::vector<T>::const_iterator begin,
	typename std::vector<T>::const_iterator end) {
	for (; begin != end; begin++) {
		v.push_back(*begin);
	}
}

my_message_t::my_message_t() {}

my_message_t::my_message_t(const my_message_t& rht) {
	str = rht.str;
}

const my_message_t& my_message_t::operator=(const my_message_t& rht) {
	str = rht.str;
	return *this;
}

void my_message_t::print(std::stringstream& ss, std::string white) {
	ss << white << "my_message_t:" << std::endl;
	white += "  ";
	ss << white << "str=" << str << std::endl;
}

my_object_t::my_object_t() {}

my_object_t::my_object_t(const my_object_t& rht) {
	n_i8 = rht.n_i8;
	n_i16 = rht.n_i16;
	n_i32 = rht.n_i32;
	n_i64 = rht.n_i64;
	n_ui8 = rht.n_ui8;
	n_ui16 = rht.n_ui16;
	n_ui32 = rht.n_ui32;
	n_ui64 = rht.n_ui64;
	n_f32 = rht.n_f32;
	n_f64 = rht.n_f64;
	push_back_all(message, rht.message.begin(), rht.message.end());
	push_back_all(test_array, rht.test_array.begin(), rht.test_array.end());
}

const my_object_t& my_object_t::operator=(const my_object_t& rht) {
	n_i8 = rht.n_i8;
	n_i16 = rht.n_i16;
	n_i32 = rht.n_i32;
	n_i64 = rht.n_i64;
	n_ui8 = rht.n_ui8;
	n_ui16 = rht.n_ui16;
	n_ui32 = rht.n_ui32;
	n_ui64 = rht.n_ui64;
	n_f32 = rht.n_f32;
	n_f64 = rht.n_f64;
	push_back_all(message, rht.message.begin(), rht.message.end());
	push_back_all(test_array, rht.test_array.begin(), rht.test_array.end());
	return *this;
}

void my_object_t::print(std::stringstream& ss, std::string white) {
	ss << white << "my_object_t:" << std::endl;
	white += "  ";
	ss << white << "n_i8=" << (uint64_t)n_i8 << std::endl;
	ss << white << "n_i16=" << (uint64_t)n_i16 << std::endl;
	ss << white << "n_i32=" << (uint64_t)n_i32 << std::endl;
	ss << white << "n_i64=" << (uint64_t)n_i64 << std::endl;
	ss << white << "n_ui8=" << (uint64_t)n_ui8 << std::endl;
	ss << white << "n_ui16=" << (uint64_t)n_ui16 << std::endl;
	ss << white << "n_ui32=" << (uint64_t)n_ui32 << std::endl;
	ss << white << "n_ui64=" << (uint64_t)n_ui64 << std::endl;
	ss << white << "n_f32=" << n_f32 << std::endl;
	ss << white << "n_f64=" << n_f64 << std::endl;
	ss << white << "message[" << std::endl;
	for (size_t i = 0; i < message.size(); i++) {
		message[i].print(ss, white + "  ");
	}
	ss << white << "]" << std::endl;
	ss << white << "test_array[" << std::endl;
	for (size_t i = 0; i < test_array.size(); i++) {
		ss << white << "  test_array=" << (uint64_t)test_array[i] << std::endl;
	}
	ss << white << "]" << std::endl;
}

krpc_object_t* my_rpc_func_proxy(my_object_t& my_obj, std::vector<my_object_t>& obj_array, const std::string& test_str, std::vector<float>& array_f32) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, marshal(my_obj));
	do {
		std::vector<my_object_t>::iterator guard = obj_array.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != obj_array.end(); guard++) {
			krpc_vector_push_back(v_, marshal(*guard));
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	krpc_object_t* test_str_string = krpc_object_create();
	krpc_string_set(test_str_string, test_str.c_str());
	krpc_vector_push_back(v, test_str_string);
	do {
		std::vector<float>::iterator guard = array_f32.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != array_f32.end(); guard++) {
			krpc_object_t* array_f32_f32 = krpc_object_create();
			krpc_number_set_f32(array_f32_f32, *guard);
			krpc_vector_push_back(v_, array_f32_f32);
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	return v;
}

int my_rpc_func_stub(krpc_object_t* o) {
	my_object_t p0;
	unmarshal(krpc_vector_get(o, 0), p0);
	std::vector<my_object_t> p1;
	do {
		krpc_object_t* v = krpc_vector_get(o, 1);
		for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {
		    my_object_t o_;
		    unmarshal(krpc_vector_get(v, i), o_);
		    p1.push_back(o_);
		}
	} while(0);
	std::string p2;
	p2 = krpc_string_get(krpc_vector_get(o, 2));
	std::vector<float> p3;
	do {
		krpc_object_t* v = krpc_vector_get(o, 3);
		for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {
		    p3.push_back(krpc_number_get_f32(krpc_vector_get(v, i)));
		}
	} while(0);
	return my_rpc_func(p0, p1, p2, p3);
}

krpc_object_t* marshal(my_message_t& o) {
	krpc_object_t* v = krpc_object_create();
	krpc_object_t* str_string = krpc_object_create();
	krpc_string_set(str_string, o.str.c_str());
	krpc_vector_push_back(v, str_string);
	return v;
}

krpc_object_t* marshal(my_object_t& o) {
	krpc_object_t* v = krpc_object_create();
	krpc_object_t* n_i8_i8 = krpc_object_create();
	krpc_number_set_i8(n_i8_i8, o.n_i8);
	krpc_vector_push_back(v, n_i8_i8);
	krpc_object_t* n_i16_i16 = krpc_object_create();
	krpc_number_set_i16(n_i16_i16, o.n_i16);
	krpc_vector_push_back(v, n_i16_i16);
	krpc_object_t* n_i32_i32 = krpc_object_create();
	krpc_number_set_i32(n_i32_i32, o.n_i32);
	krpc_vector_push_back(v, n_i32_i32);
	krpc_object_t* n_i64_i64 = krpc_object_create();
	krpc_number_set_i64(n_i64_i64, o.n_i64);
	krpc_vector_push_back(v, n_i64_i64);
	krpc_object_t* n_ui8_ui8 = krpc_object_create();
	krpc_number_set_ui8(n_ui8_ui8, o.n_ui8);
	krpc_vector_push_back(v, n_ui8_ui8);
	krpc_object_t* n_ui16_ui16 = krpc_object_create();
	krpc_number_set_ui16(n_ui16_ui16, o.n_ui16);
	krpc_vector_push_back(v, n_ui16_ui16);
	krpc_object_t* n_ui32_ui32 = krpc_object_create();
	krpc_number_set_ui32(n_ui32_ui32, o.n_ui32);
	krpc_vector_push_back(v, n_ui32_ui32);
	krpc_object_t* n_ui64_ui64 = krpc_object_create();
	krpc_number_set_ui64(n_ui64_ui64, o.n_ui64);
	krpc_vector_push_back(v, n_ui64_ui64);
	krpc_object_t* n_f32_f32 = krpc_object_create();
	krpc_number_set_f32(n_f32_f32, o.n_f32);
	krpc_vector_push_back(v, n_f32_f32);
	krpc_object_t* n_f64_f64 = krpc_object_create();
	krpc_number_set_f64(n_f64_f64, o.n_f64);
	krpc_vector_push_back(v, n_f64_f64);
	do {
		std::vector<my_message_t>::iterator guard = o.message.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != o.message.end(); guard++) {
			krpc_vector_push_back(v_, marshal(*guard));
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	do {
		std::vector<int32_t>::iterator guard = o.test_array.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != o.test_array.end(); guard++) {
			krpc_object_t* test_array_i32 = krpc_object_create();
			krpc_number_set_i32(test_array_i32, *guard);
			krpc_vector_push_back(v_, test_array_i32);
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	return v;
}

bool unmarshal(krpc_object_t* v, my_message_t& o) {
	o.str = krpc_string_get(krpc_vector_get(v, 0));
	return true;
}

bool unmarshal(krpc_object_t* v, my_object_t& o) {
	o.n_i8 = krpc_number_get_i8(krpc_vector_get(v, 0));
	o.n_i16 = krpc_number_get_i16(krpc_vector_get(v, 1));
	o.n_i32 = krpc_number_get_i32(krpc_vector_get(v, 2));
	o.n_i64 = krpc_number_get_i64(krpc_vector_get(v, 3));
	o.n_ui8 = krpc_number_get_ui8(krpc_vector_get(v, 4));
	o.n_ui16 = krpc_number_get_ui16(krpc_vector_get(v, 5));
	o.n_ui32 = krpc_number_get_ui32(krpc_vector_get(v, 6));
	o.n_ui64 = krpc_number_get_ui64(krpc_vector_get(v, 7));
	o.n_f32 = krpc_number_get_f32(krpc_vector_get(v, 8));
	o.n_f64 = krpc_number_get_f64(krpc_vector_get(v, 9));
	do {
		krpc_object_t* v_ = 0;
		v_ = krpc_vector_get(v, 10);
		for (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {
		    my_message_t o_;
		    unmarshal(krpc_vector_get(v_, i), o_);
		    o.message.push_back(o_);
		}
	} while(0);
	do {
		krpc_object_t* v_ = 0;
		v_ = krpc_vector_get(v, 11);
		for (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {
		    o.test_array.push_back(krpc_number_get_i32(krpc_vector_get(v_, i)));
		}
	} while(0);
	return true;
}

rpc_sample_t* rpc_sample_t::_instance = 0;

rpc_sample_t::rpc_sample_t() {
	_rpc = krpc_create();
	krpc_add_cb(_rpc, 1, my_rpc_func_stub);
}

rpc_sample_t::~rpc_sample_t() {
	krpc_destroy(_rpc);
}

rpc_sample_t* rpc_sample_t::instance() {
	if (!_instance) {
		_instance = new rpc_sample_t();
	}
	return _instance;
}

void rpc_sample_t::finalize() {
	if (_instance) {
		delete _instance;
	}
}

int rpc_sample_t::rpc_proc(stream_t* stream) {
	return krpc_proc(_rpc, stream);
}

int rpc_sample_t::my_rpc_func(stream_t* stream, my_object_t& my_obj, std::vector<my_object_t>& obj_array, const std::string& test_str, std::vector<float>& array_f32) {
	return krpc_call(_rpc, stream, 1, my_rpc_func_proxy(my_obj, obj_array, test_str, array_f32));
}

