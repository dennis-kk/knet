                {{@value_type_name}} o__;
                unmarshal(v_, o__);
                p{{$index}}.insert(std::make_pair({{@key_get_func}}(k_), o__));
