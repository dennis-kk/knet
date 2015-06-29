//
// KRPC - Generated code, *DO NOT CHANGE*
//

#include <sstream>
#include "rpc_sample.h"

namespace rpc_sample {

template<typename T>
void push_back_all(std::vector<T>& v, typename std::vector<T>::const_iterator begin,
	typename std::vector<T>::const_iterator end) {
	for (; begin != end; begin++) {
		v.push_back(*begin);
	}
}

my_object_other_t::my_object_other_t() {}

my_object_other_t::my_object_other_t(const my_object_other_t& rht) {
	push_back_all(objects, rht.objects.begin(), rht.objects.end());
	push_back_all(i32_array, rht.i32_array.begin(), rht.i32_array.end());
	push_back_all(str_array, rht.str_array.begin(), rht.str_array.end());
}

const my_object_other_t& my_object_other_t::operator=(const my_object_other_t& rht) {
	push_back_all(objects, rht.objects.begin(), rht.objects.end());
	push_back_all(i32_array, rht.i32_array.begin(), rht.i32_array.end());
	push_back_all(str_array, rht.str_array.begin(), rht.str_array.end());
	return *this;
}

void my_object_other_t::print(std::stringstream& ss, std::string white) {
	ss << white << "my_object_other_t:" << std::endl;
	white += "  ";
	ss << white << "objects[" << std::endl;
	for (size_t i = 0; i < objects.size(); i++) {
		objects[i].print(ss, white + "  ");
	}
	ss << white << "]" << std::endl;
	ss << white << "i32_array[" << std::endl;
	for (size_t i = 0; i < i32_array.size(); i++) {
		ss << white << "  i32_array=" << (uint64_t)i32_array[i] << std::endl;
	}
	ss << white << "]" << std::endl;
	ss << white << "str_array[" << std::endl;
	for (size_t i = 0; i < str_array.size(); i++) {
		ss << white << "  str_array=" << str_array[i] << std::endl;
	}
	ss << white << "]" << std::endl;
}

my_object_t::my_object_t() {}

my_object_t::my_object_t(const my_object_t& rht) {
	ni8 = rht.ni8;
	ni16 = rht.ni16;
	ni32 = rht.ni32;
	ni64 = rht.ni64;
	nui8 = rht.nui8;
	nui16 = rht.nui16;
	nui32 = rht.nui32;
	nui64 = rht.nui64;
	nf32 = rht.nf32;
	nf64 = rht.nf64;
	str = rht.str;
}

const my_object_t& my_object_t::operator=(const my_object_t& rht) {
	ni8 = rht.ni8;
	ni16 = rht.ni16;
	ni32 = rht.ni32;
	ni64 = rht.ni64;
	nui8 = rht.nui8;
	nui16 = rht.nui16;
	nui32 = rht.nui32;
	nui64 = rht.nui64;
	nf32 = rht.nf32;
	nf64 = rht.nf64;
	str = rht.str;
	return *this;
}

void my_object_t::print(std::stringstream& ss, std::string white) {
	ss << white << "my_object_t:" << std::endl;
	white += "  ";
	ss << white << "ni8=" << (uint64_t)ni8 << std::endl;
	ss << white << "ni16=" << (uint64_t)ni16 << std::endl;
	ss << white << "ni32=" << (uint64_t)ni32 << std::endl;
	ss << white << "ni64=" << (uint64_t)ni64 << std::endl;
	ss << white << "nui8=" << (uint64_t)nui8 << std::endl;
	ss << white << "nui16=" << (uint64_t)nui16 << std::endl;
	ss << white << "nui32=" << (uint64_t)nui32 << std::endl;
	ss << white << "nui64=" << (uint64_t)nui64 << std::endl;
	ss << white << "nf32=" << nf32 << std::endl;
	ss << white << "nf64=" << nf64 << std::endl;
	ss << white << "str=" << str << std::endl;
}

krpc_object_t* my_rpc_func1_proxy(my_object_t& my_obj) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, marshal(my_obj));
	return v;
}

int my_rpc_func1_stub(krpc_object_t* o) {
	my_object_t p0;
	unmarshal(krpc_vector_get(o, 0), p0);
	return my_rpc_func1(p0);
}

krpc_object_t* my_rpc_func2_proxy(std::vector<my_object_t>& my_objs, int8_t my_i8) {
	krpc_object_t* v = krpc_object_create();
	do {
		std::vector<my_object_t>::iterator guard = my_objs.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != my_objs.end(); guard++) {
			krpc_vector_push_back(v_, marshal((*guard)));
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	krpc_object_t* my_i8_i8 = krpc_object_create();
	krpc_number_set_i8(my_i8_i8, my_i8);
	krpc_vector_push_back(v, my_i8_i8);
	return v;
}

int my_rpc_func2_stub(krpc_object_t* o) {
	std::vector<my_object_t> p0;
	do {
		krpc_object_t* v = krpc_vector_get(o, 0);
		for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {
		    my_object_t o_;
		    unmarshal(krpc_vector_get(v, i), o_);
		    p0.push_back(o_);
		}
	} while(0);
	int8_t p1;
	p1 = krpc_number_get_i8(krpc_vector_get(o, 1));
	return my_rpc_func2(p0, p1);
}

krpc_object_t* my_rpc_func3_proxy(const std::string& my_str, int8_t my_i8) {
	krpc_object_t* v = krpc_object_create();
	krpc_object_t* my_str_string = krpc_object_create();
	krpc_string_set(my_str_string, my_str.c_str());
	krpc_vector_push_back(v, my_str_string);
	krpc_object_t* my_i8_i8 = krpc_object_create();
	krpc_number_set_i8(my_i8_i8, my_i8);
	krpc_vector_push_back(v, my_i8_i8);
	return v;
}

int my_rpc_func3_stub(krpc_object_t* o) {
	std::string p0;
	p0 = krpc_string_get(krpc_vector_get(o, 0));
	int8_t p1;
	p1 = krpc_number_get_i8(krpc_vector_get(o, 1));
	return my_rpc_func3(p0, p1);
}

krpc_object_t* marshal(my_object_other_t& o) {
	krpc_object_t* v = krpc_object_create();
	do {
		std::vector<my_object_t>::iterator guard = o.objects.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != o.objects.end(); guard++) {
			krpc_vector_push_back(v_, marshal((*guard)));
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	do {
		std::vector<int32_t>::iterator guard = o.i32_array.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != o.i32_array.end(); guard++) {
			krpc_object_t* i32_array_i32 = krpc_object_create();
			krpc_number_set_i32(i32_array_i32, (*guard));
			krpc_vector_push_back(v_, i32_array_i32);
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	do {
		std::vector<std::string>::iterator guard = o.str_array.begin();
		krpc_object_t* v_ = krpc_object_create();
		krpc_vector_clear(v_);
		for(; guard != o.str_array.end(); guard++) {
			krpc_object_t* str_array_string = krpc_object_create();
			krpc_string_set(str_array_string, (*guard).c_str());
			krpc_vector_push_back(v_, str_array_string);
		}
		krpc_vector_push_back(v, v_);
	} while(0);
	return v;
}

krpc_object_t* marshal(my_object_t& o) {
	krpc_object_t* v = krpc_object_create();
	krpc_object_t* ni8_i8 = krpc_object_create();
	krpc_number_set_i8(ni8_i8, o.ni8);
	krpc_vector_push_back(v, ni8_i8);
	krpc_object_t* ni16_i16 = krpc_object_create();
	krpc_number_set_i16(ni16_i16, o.ni16);
	krpc_vector_push_back(v, ni16_i16);
	krpc_object_t* ni32_i32 = krpc_object_create();
	krpc_number_set_i32(ni32_i32, o.ni32);
	krpc_vector_push_back(v, ni32_i32);
	krpc_object_t* ni64_i64 = krpc_object_create();
	krpc_number_set_i64(ni64_i64, o.ni64);
	krpc_vector_push_back(v, ni64_i64);
	krpc_object_t* nui8_ui8 = krpc_object_create();
	krpc_number_set_ui8(nui8_ui8, o.nui8);
	krpc_vector_push_back(v, nui8_ui8);
	krpc_object_t* nui16_ui16 = krpc_object_create();
	krpc_number_set_ui16(nui16_ui16, o.nui16);
	krpc_vector_push_back(v, nui16_ui16);
	krpc_object_t* nui32_ui32 = krpc_object_create();
	krpc_number_set_ui32(nui32_ui32, o.nui32);
	krpc_vector_push_back(v, nui32_ui32);
	krpc_object_t* nui64_ui64 = krpc_object_create();
	krpc_number_set_ui64(nui64_ui64, o.nui64);
	krpc_vector_push_back(v, nui64_ui64);
	krpc_object_t* nf32_f32 = krpc_object_create();
	krpc_number_set_f32(nf32_f32, o.nf32);
	krpc_vector_push_back(v, nf32_f32);
	krpc_object_t* nf64_f64 = krpc_object_create();
	krpc_number_set_f64(nf64_f64, o.nf64);
	krpc_vector_push_back(v, nf64_f64);
	krpc_object_t* str_string = krpc_object_create();
	krpc_string_set(str_string, o.str.c_str());
	krpc_vector_push_back(v, str_string);
	return v;
}

bool unmarshal(krpc_object_t* v, my_object_other_t& o) {
	do {
		krpc_object_t* v_ = 0;
		v_ = krpc_vector_get(v, 0);
		for (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {
		    my_object_t o_;
		    unmarshal(krpc_vector_get(v_, i), o_);
		    o.objects.push_back(o_);
		}
	} while(0);
	do {
		krpc_object_t* v_ = 0;
		v_ = krpc_vector_get(v, 1);
		for (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {
		    o.i32_array.push_back(krpc_number_get_i32(krpc_vector_get(v_, i)));
		}
	} while(0);
	do {
		krpc_object_t* v_ = 0;
		v_ = krpc_vector_get(v, 2);
		for (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {
		    o.str_array.push_back(krpc_string_get(krpc_vector_get(v_, i)));
		}
	} while(0);
	return true;
}

bool unmarshal(krpc_object_t* v, my_object_t& o) {
	o.ni8 = krpc_number_get_i8(krpc_vector_get(v, 0));
	o.ni16 = krpc_number_get_i16(krpc_vector_get(v, 1));
	o.ni32 = krpc_number_get_i32(krpc_vector_get(v, 2));
	o.ni64 = krpc_number_get_i64(krpc_vector_get(v, 3));
	o.nui8 = krpc_number_get_ui8(krpc_vector_get(v, 4));
	o.nui16 = krpc_number_get_ui16(krpc_vector_get(v, 5));
	o.nui32 = krpc_number_get_ui32(krpc_vector_get(v, 6));
	o.nui64 = krpc_number_get_ui64(krpc_vector_get(v, 7));
	o.nf32 = krpc_number_get_f32(krpc_vector_get(v, 8));
	o.nf64 = krpc_number_get_f64(krpc_vector_get(v, 9));
	o.str = krpc_string_get(krpc_vector_get(v, 10));
	return true;
}

rpc_sample_t* rpc_sample_t::_instance = 0;

rpc_sample_t::rpc_sample_t() {
	_rpc = krpc_create();
	krpc_add_cb(_rpc, 1, my_rpc_func1_stub);
	krpc_add_cb(_rpc, 2, my_rpc_func2_stub);
	krpc_add_cb(_rpc, 3, my_rpc_func3_stub);
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

int rpc_sample_t::my_rpc_func1(stream_t* stream, my_object_t& my_obj) {
	krpc_object_t* o = my_rpc_func1_proxy(my_obj);
	int error = krpc_call(_rpc, stream, 1, o);
	krpc_object_destroy(o);
	return error;
}

int rpc_sample_t::my_rpc_func2(stream_t* stream, std::vector<my_object_t>& my_objs, int8_t my_i8) {
	krpc_object_t* o = my_rpc_func2_proxy(my_objs, my_i8);
	int error = krpc_call(_rpc, stream, 2, o);
	krpc_object_destroy(o);
	return error;
}

int rpc_sample_t::my_rpc_func3(stream_t* stream, const std::string& my_str, int8_t my_i8) {
	krpc_object_t* o = my_rpc_func3_proxy(my_str, my_i8);
	int error = krpc_call(_rpc, stream, 3, o);
	krpc_object_destroy(o);
	return error;
}

}

