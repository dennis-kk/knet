    std::map<{{@key_type}}, {{@value_type}}> p{{$index}};
    do {
        krpc_object_t* m_ = krpc_vector_get(o, {{$index}});
        krpc_object_t* k_ = 0;
        krpc_object_t* v_ = 0;
        if (krpc_map_get_first(m_, &k_, &v_)) {
            