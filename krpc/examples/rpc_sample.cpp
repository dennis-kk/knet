//
// KRPC - Generated code, *DO NOT CHANGE*
//

#include <sstream>
#include "rpc_sample.h"

namespace rpc_sample {

/////////////////////////////////////////////////////////
rpc_sample_t* rpc_sample_t::_instance = 0;

/////////////////////////////////////////////////////////
template<typename T>
void push_back_all(std::vector<T>& v, typename std::vector<T>::const_iterator begin,
	typename std::vector<T>::const_iterator end) {
	for (; begin != end; begin++) {
		v.push_back(*begin);
	}
}

template<typename K, typename V>
void insert_all(std::map<K, V>& m, typename std::map<K, V>::const_iterator begin,
	typename std::map<K, V>::const_iterator end) {
	for (; begin != end; begin++) {
		m.insert(std::make_pair(begin->first, begin->second));
	}
}

/////////////////////////////////////////////////////////
void	krpc_member_set(krpc_object_t* t, const std::string& node){
	krpc_string_set_s(t, node.c_str(), node.length());
}
void	krpc_member_set(krpc_object_t* t, const char* node){
	krpc_string_set(t, node);
}
void	krpc_member_set(krpc_object_t* t, const char* node, uint16_t size){
	krpc_string_set_s(t, node, size);
}
void	krpc_member_get(krpc_object_t* t, std::string& node){
	node = std::string(krpc_string_get(t), krpc_string_get_size(t));
}
void	krpc_member_get(krpc_object_t* t, const char*& node){
	node = krpc_string_get(t);
}

KRPC_MEMBER_SET(int8_t, i8);
KRPC_MEMBER_SET(int16_t, i16);
KRPC_MEMBER_SET(int32_t, i32);
KRPC_MEMBER_SET(int64_t, i64);
KRPC_MEMBER_SET(uint8_t, ui8);
KRPC_MEMBER_SET(uint16_t, ui16);
KRPC_MEMBER_SET(uint32_t, ui32);
KRPC_MEMBER_SET(uint64_t, ui64);
KRPC_MEMBER_SET(float32_t, f32);
KRPC_MEMBER_SET(float64_t, f64);

KRPC_MEMBER_GET(int8_t, i8);
KRPC_MEMBER_GET(int16_t, i16);
KRPC_MEMBER_GET(int32_t, i32);
KRPC_MEMBER_GET(int64_t, i64);
KRPC_MEMBER_GET(uint8_t, ui8);
KRPC_MEMBER_GET(uint16_t, ui16);
KRPC_MEMBER_GET(uint32_t, ui32);
KRPC_MEMBER_GET(uint64_t, ui64);
KRPC_MEMBER_GET(float32_t, f32);
KRPC_MEMBER_GET(float64_t, f64);

KRPC_MARSHAL_COMM(int8_t);
KRPC_MARSHAL_COMM(int16_t);
KRPC_MARSHAL_COMM(int32_t);
KRPC_MARSHAL_COMM(int64_t);
KRPC_MARSHAL_COMM(uint8_t);
KRPC_MARSHAL_COMM(uint16_t);
KRPC_MARSHAL_COMM(uint32_t);
KRPC_MARSHAL_COMM(uint64_t);
KRPC_MARSHAL_COMM(float32_t);
KRPC_MARSHAL_COMM(float64_t);
KRPC_MARSHAL_COMM(const std::string&);
KRPC_MARSHAL_COMM(const char*);

KRPC_UNMARSHAL_COMM(int8_t);
KRPC_UNMARSHAL_COMM(int16_t);
KRPC_UNMARSHAL_COMM(int32_t);
KRPC_UNMARSHAL_COMM(int64_t);
KRPC_UNMARSHAL_COMM(uint8_t);
KRPC_UNMARSHAL_COMM(uint16_t);
KRPC_UNMARSHAL_COMM(uint32_t);
KRPC_UNMARSHAL_COMM(uint64_t);
KRPC_UNMARSHAL_COMM(float32_t);
KRPC_UNMARSHAL_COMM(float64_t);
KRPC_UNMARSHAL_COMM(std::string);
KRPC_UNMARSHAL_COMM(const char*);
/////////////////////////////////////////////////////////
template<typename T>
krpc_object_t* 	krpc_marshal(T& node){
	return marshal(node);
}

template<typename T1, typename T2>
krpc_object_t* krpc_marshal(std::map<T1, T2>& node){
	krpc_object_t* pNode = krpc_object_create();
	krpc_map_clear(pNode);
	for(typename std::map<T1, T2>::iterator iter = node.begin(); iter != node.end(); iter++) {
		krpc_map_insert(pNode, krpc_marshal(iter->first), krpc_marshal(iter->second));
	}
	return pNode;
}

template<typename T>
krpc_object_t* krpc_marshal(std::vector<T>& node){
	krpc_object_t* pNode = krpc_object_create();
	krpc_vector_clear(pNode);
	for(size_t i=0;i<node.size();i++) {
		krpc_vector_push_back(pNode, krpc_marshal(node[i]));
	}
	return pNode;
}

/////////////////////////////////////////////////////////
template<typename T>
bool krpc_unmarshal(krpc_object_t* m_, T& node){
	return unmarshal(m_, node);
}

template<typename T1, typename T2>
bool krpc_unmarshal(krpc_object_t* m_, std::map<T1, T2>& node){
	krpc_object_t* k_ = NULL;
	krpc_object_t* v_ = NULL;
	T1 key_;
	T2 val_;
	for (krpc_map_get_first(m_, &k_, &v_); (k_) && (v_); krpc_map_next(m_, &k_, &v_)) {
		if(krpc_unmarshal(k_, key_) && krpc_unmarshal(v_, val_)){
			node.insert(std::make_pair(key_, val_));
		}
		k_ = v_ = NULL;
	}
	return true;
}

template<typename T>
bool krpc_unmarshal(krpc_object_t* m_, std::vector<T>& node){
	T o_;
	uint32_t nSize = krpc_vector_get_size(m_);
	for (uint32_t i = 0; i < nSize; i++) {
		if(krpc_unmarshal(krpc_vector_get(m_, i), o_)){
			node.push_back(o_);
		}
	}
	return true;
}
/////////////////////////////////////////////////////////
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

int rpc_sample_t::rpc_proc(kstream_t* stream) {
	return krpc_proc(_rpc, stream);
}

krpc_t* rpc_sample_t::get_rpc() {
	return _rpc;
}

int rpc_sample_t::my_rpc_func1(kstream_t* stream, my_object_t& my_obj) {
	krpc_object_t* o = my_rpc_func1_proxy(my_obj);
	int error = krpc_call(_rpc, stream, 1, o);
	krpc_object_destroy(o);
	return error;
}

int rpc_sample_t::my_rpc_func2(kstream_t* stream, std::vector<my_object_t>& my_objs, int8_t my_i8) {
	krpc_object_t* o = my_rpc_func2_proxy(my_objs, my_i8);
	int error = krpc_call(_rpc, stream, 2, o);
	krpc_object_destroy(o);
	return error;
}

int rpc_sample_t::my_rpc_func3(kstream_t* stream, const std::string& my_str, int8_t my_i8) {
	krpc_object_t* o = my_rpc_func3_proxy(my_str, my_i8);
	int error = krpc_call(_rpc, stream, 3, o);
	krpc_object_destroy(o);
	return error;
}

krpc_object_t* marshal(my_object_other_t& o) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, krpc_marshal(o.string_string_table));
	return v;
}

krpc_object_t* marshal(my_object_t& o) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, krpc_marshal(o.ni8));
	krpc_vector_push_back(v, krpc_marshal(o.ni16));
	krpc_vector_push_back(v, krpc_marshal(o.ni32));
	krpc_vector_push_back(v, krpc_marshal(o.ni64));
	krpc_vector_push_back(v, krpc_marshal(o.nui8));
	krpc_vector_push_back(v, krpc_marshal(o.nui16));
	krpc_vector_push_back(v, krpc_marshal(o.nui32));
	krpc_vector_push_back(v, krpc_marshal(o.nui64));
	krpc_vector_push_back(v, krpc_marshal(o.nf32));
	krpc_vector_push_back(v, krpc_marshal(o.nf64));
	krpc_vector_push_back(v, krpc_marshal(o.str));
	krpc_vector_push_back(v, krpc_marshal(o.int_string_table));
	krpc_vector_push_back(v, krpc_marshal(o.int_object_table));
	return v;
}

bool unmarshal(krpc_object_t* v, my_object_other_t& o) {
	krpc_unmarshal(krpc_vector_get(v, 0), o.string_string_table);
	return true;
}

bool unmarshal(krpc_object_t* v, my_object_t& o) {
	krpc_unmarshal(krpc_vector_get(v, 0), o.ni8);
	krpc_unmarshal(krpc_vector_get(v, 1), o.ni16);
	krpc_unmarshal(krpc_vector_get(v, 2), o.ni32);
	krpc_unmarshal(krpc_vector_get(v, 3), o.ni64);
	krpc_unmarshal(krpc_vector_get(v, 4), o.nui8);
	krpc_unmarshal(krpc_vector_get(v, 5), o.nui16);
	krpc_unmarshal(krpc_vector_get(v, 6), o.nui32);
	krpc_unmarshal(krpc_vector_get(v, 7), o.nui64);
	krpc_unmarshal(krpc_vector_get(v, 8), o.nf32);
	krpc_unmarshal(krpc_vector_get(v, 9), o.nf64);
	krpc_unmarshal(krpc_vector_get(v, 10), o.str);
	krpc_unmarshal(krpc_vector_get(v, 11), o.int_string_table);
	krpc_unmarshal(krpc_vector_get(v, 12), o.int_object_table);
	return true;
}

krpc_object_t* my_rpc_func1_proxy(my_object_t& my_obj) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, krpc_marshal(my_obj));
	return v;
}

krpc_object_t* my_rpc_func2_proxy(std::vector<my_object_t>& my_objs, int8_t my_i8) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, krpc_marshal(my_objs));
	krpc_vector_push_back(v, krpc_marshal(my_i8));
	return v;
}

krpc_object_t* my_rpc_func3_proxy(const std::string& my_str, int8_t my_i8) {
	krpc_object_t* v = krpc_object_create();
	krpc_vector_push_back(v, krpc_marshal(my_str));
	krpc_vector_push_back(v, krpc_marshal(my_i8));
	return v;
}

int my_rpc_func1_stub(krpc_object_t* o) {
	my_object_t p0;
	krpc_unmarshal(krpc_vector_get(o, 0), p0);
	return my_rpc_func1(p0);
}

int my_rpc_func2_stub(krpc_object_t* o) {
	std::vector<my_object_t> p0;
	krpc_unmarshal(krpc_vector_get(o, 0), p0);
	int8_t p1;
	krpc_unmarshal(krpc_vector_get(o, 1), p1);
	return my_rpc_func2(p0, p1);
}

int my_rpc_func3_stub(krpc_object_t* o) {
	std::string p0;
	krpc_unmarshal(krpc_vector_get(o, 0), p0);
	int8_t p1;
	krpc_unmarshal(krpc_vector_get(o, 1), p1);
	return my_rpc_func3(p0, p1);
}

my_object_other_t::my_object_other_t() {
}

my_object_other_t::my_object_other_t(const my_object_other_t& rht) {
	insert_all(string_string_table, rht.string_string_table.begin(), rht.string_string_table.end());
}

const my_object_other_t& my_object_other_t::operator=(const my_object_other_t& rht) {
	insert_all(string_string_table, rht.string_string_table.begin(), rht.string_string_table.end());
	return *this;
}

void my_object_other_t::print(std::stringstream& ss, std::string white) {
	ss << white << "my_object_other_t:" << std::endl;
	white += "  ";
	ss << white << "string_string_table<" << std::endl;
	do {
		std::map<std::string, std::string>::iterator guard = string_string_table.begin();
		for (; guard != string_string_table.end(); guard++) {
			ss << white + "  " << "key:" << guard->first << std::endl;
			ss << white + "  " << "value:" << guard->second << std::endl;
		}
	} while(0);
	ss << white << ">" << std::endl;
}

my_object_t::my_object_t() {
}

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
	insert_all(int_string_table, rht.int_string_table.begin(), rht.int_string_table.end());
	insert_all(int_object_table, rht.int_object_table.begin(), rht.int_object_table.end());
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
	insert_all(int_string_table, rht.int_string_table.begin(), rht.int_string_table.end());
	insert_all(int_object_table, rht.int_object_table.begin(), rht.int_object_table.end());
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
	ss << white << "int_string_table<" << std::endl;
	do {
		std::map<int8_t, std::string>::iterator guard = int_string_table.begin();
		for (; guard != int_string_table.end(); guard++) {
			ss << white + "  " << "key:" << (uint64_t)guard->first << std::endl;
			ss << white + "  " << "value:" << guard->second << std::endl;
		}
	} while(0);
	ss << white << ">" << std::endl;
	ss << white << "int_object_table<" << std::endl;
	do {
		std::map<int8_t, my_object_other_t>::iterator guard = int_object_table.begin();
		for (; guard != int_object_table.end(); guard++) {
			ss << white + "  " << "key:" << (uint64_t)guard->first << std::endl;
			ss << white + "  " << "value:";
			ss << std::endl;
			guard->second.print(ss, white + "    ");
		}
	} while(0);
	ss << white << ">" << std::endl;
}

} // namespace rpc_sample

