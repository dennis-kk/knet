//
// KRPC - Generated code, *DO NOT CHANGE*
//

#include <sstream>
#include "{{@file_name}}.h"

namespace {{@file_name}} {

/////////////////////////////////////////////////////////
{{@file_name}}_t* {{@file_name}}_t::_instance = 0;

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
	for(std::map<T1, T2>::iterator iter = node.begin(); iter != node.end(); iter++) {
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
