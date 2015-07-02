{{@object_type}} o_;
            unmarshal(v_, o_);
            p{{$index}}.insert(std::make_pair({{@key_get_func}}(k_), o_));
