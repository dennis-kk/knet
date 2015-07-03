};

inline static {{@file_name}}_t* {{@file_name}}_ptr() {
	return {{@file_name}}_t::instance();
}

} // namespace {{@file_name}}

#endif // _krpc_{{@file_name}}_h_

