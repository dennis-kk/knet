{{@field_find_type_name}} o_;
            unmarshal(krpc_vector_get(v, i), o_);
            p{{$index}}.push_back(o_);
