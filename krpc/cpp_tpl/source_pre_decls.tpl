#include <sstream>
#include "{{@file_name}}.h"

namespace {{@file_name}} {

{{@file_name}}_t* {{@file_name}}_t::_instance = 0;

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

