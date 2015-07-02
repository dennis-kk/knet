    do {
        krpc_object_t* m_ = krpc_vector_get(v, {{$index}});
        krpc_object_t* k_ = 0;
        krpc_object_t* v_ = 0;
        for (krpc_map_get_first(m_, &k_, &v_); (k_) && (v_); krpc_map_next(m_, &k_, &v_)) {
