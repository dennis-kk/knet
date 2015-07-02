            {{@value_type}} o_;
            unmarshal(v_, o_);
            o.{{@name}}.insert(std::make_pair({{@get_func_name}}(k_), o_));
