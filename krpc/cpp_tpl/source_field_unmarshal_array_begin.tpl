    std::vector<{{@field_find_type_name}}> p{{$index}};
     do {
         krpc_object_t* v = krpc_vector_get(o, {{$index}});
         for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {
