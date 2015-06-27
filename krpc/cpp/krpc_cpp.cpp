/*
 * Copyright (c) 2014-2015, dennis wang
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL dennis wang BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include "krpc_cpp.h"
#include "krpc.h"
#include "krpc_parser.h"
#include "krpc_exception.h"

krpc_gen_cpp_t::krpc_gen_cpp_t(krpc_gen_t* rpc_gen)
: _rpc_gen(rpc_gen),
  _rpc_id(1) {
}

krpc_gen_cpp_t::~krpc_gen_cpp_t() {
}

void krpc_gen_cpp_t::lang_gen_code() {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    krpc_ostream_t header(options["dir"] + options["name"] + ".h");
    krpc_ostream_t source(options["dir"] + options["name"] + ".cpp");
    header.write("//\n"
                 "// Generated code, DO NOT modify\n"
                 "//\n\n");
    source.write("//\n"
                 "// Generated code, DO NOT modify\n"
                 "//\n\n");
    header.replace("#ifndef _krpc_@file_name@_h_\n"
                   "#define _krpc_@file_name@_h_\n\n",
                   options["name"]);
    lang_gen_includes(header); // 头文件
    lang_gen_attributes_pre_decls(header); // 预先声明
    lang_gen_attributes(header, source); // 属性对象
    lang_gen_rpc_call_decls(header); // RPC声明
    lang_gen_rpc_call_impls(source); // RPC定义
    lang_gen_attribute_marshal_method_impls(source); // marshal方法
    lang_gen_attribute_unmarshal_method_impls(source); // unmarshal方法
    lang_gen_framework(header, source); // 入口框架
    header.replace("#endif // _krpc_@file_name@_h_\n", options["name"]);
}

void krpc_gen_cpp_t::lang_gen_includes(krpc_ostream_t& header) {
    header.write("#include <cstdint>\n"
                 "#include <string>\n" 
                 "#include <vector>\n"
                 "#include \"knet.h\"\n\n");
}

void krpc_gen_cpp_t::lang_gen_framework(krpc_ostream_t& header, krpc_ostream_t& source) {
    lang_gen_framework_decls(header);
    lang_gen_framework_impls(source);
}

void krpc_gen_cpp_t::lang_gen_framework_decls(krpc_ostream_t& header) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    header.replace("class @name@_t {\n"
                   "public:\n"
                   "    ~@name@_t();\n"
                   "    static @name@_t* instance();\n"
                   "    static void finalize();\n"
                   "    int rpc_proc(stream_t* stream); \n",
                   options["name"]);
    // RPC包装方法
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call = parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        header << "    int " << rpc_call->first << "(stream_t* stream, ";
        krpc_attribute_t::field_list_t::iterator field = rpc_call->second->get_attribute()->get_field_list().begin();
        size_t size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header.write(");\n");
    }
    // 私有成员
    header.replace("private:\n"
                   "    @name@_t();\n"
                   "    @name@_t(const @name@_t&);\n"
                   "    static @name@_t* _instance;\n"
                   "    krpc_t* _rpc;\n"
                   "};\n\n",
                   options["name"]);
}

void krpc_gen_cpp_t::lang_gen_framework_impls(krpc_ostream_t& source) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    source.replace("@name@_t* @name@_t::_instance = 0;\n\n"
                   "@name@_t::@name@_t() {\n"
                   "    _rpc = krpc_create();\n",
                   options["name"]);
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call = parser->get_rpc_calls().begin();
    // 注册stub
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++, _rpc_id++) {
        source << "    " << "krpc_add_cb(_rpc, " << _rpc_id << ", " << rpc_call->first << "_stub);\n";
    }
    source.replace("}\n\n"
                 "@name@_t::~@name@_t() {\n"
                 "    krpc_destroy(_rpc);\n"
                 "}\n\n"
                 "@name@_t* @name@_t::instance() {\n"
                 "    if (!_instance) {\n"
                 "        _instance = new @name@_t();\n"
                 "    }\n"
                 "    return _instance;\n"
                 "}\n\n"
                 "void @name@_t::finalize() {\n"
                 "    if (_instance) {\n"
                 "        delete _instance;\n"
                 "    }\n"
                 "}\n\n"
                 "int @name@_t::rpc_proc(stream_t* stream) {\n"
                 "    return krpc_proc(_rpc, stream);\n"
                 "}\n\n",
                 options["name"]);
    // RPC包装方法
    rpc_call = parser->get_rpc_calls().begin();
    for (int rpcid = 1; rpc_call != parser->get_rpc_calls().end(); rpc_call++, rpcid++) {
        source << "int " << options["name"] << "_t::" << rpc_call->first << "(stream_t* stream, ";
        krpc_attribute_t::field_list_t::iterator field = rpc_call->second->get_attribute()->get_field_list().begin();
        size_t size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            lang_gen_rpc_call_param_decl(source, *field);
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << ") {\n"
               << "    return krpc_call(_rpc, stream, " << rpcid << ", " << rpc_call->first << "_proxy(";
        field = rpc_call->second->get_attribute()->get_field_list().begin();
        size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            source << (*field)->get_field_name();
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << "));\n"
               << "}\n\n";
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_decls(krpc_ostream_t& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call = parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        header << "krpc_object_t* " << rpc_call->first << "_proxy(";
        krpc_attribute_t::field_list_t::iterator field = rpc_call->second->get_attribute()->get_field_list().begin();
        size_t size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header << ");\n"
               << "int " << rpc_call->first << "_stub(krpc_object_t* o);\n"
               << "int " << rpc_call->first << "(";
        field = rpc_call->second->get_attribute()->get_field_list().begin();
        size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header << ");\n";
    }
    header << "\n";
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call = parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        source << "krpc_object_t* " << rpc_call->first << "_proxy(";
        krpc_attribute_t::field_list_t::iterator field = rpc_call->second->get_attribute()->get_field_list().begin();
        size_t size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            lang_gen_rpc_call_param_decl(source, *field);
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << ") {\n";
        lang_gen_rpc_call_impl_proxy(rpc_call->second->get_attribute(), source);
        source << "    return v;\n"
               << "}\n\n";

        source << "int " << rpc_call->first << "_stub(krpc_object_t* o) {\n";
        lang_gen_rpc_call_impl_stub(rpc_call->second->get_attribute(), source, rpc_call->first);
        source << "}\n\n";
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impl_proxy(krpc_attribute_t* attribute, krpc_ostream_t& source) {
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    if (size > 0) {
        source << "    krpc_object_t* v = krpc_object_create();\n";
    }
    for (; field != attribute->get_field_list().end(); field++) {
        lang_gen_field_marshal_impl(*field, source, true);
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impl_stub(krpc_attribute_t* attribute, krpc_ostream_t& source, const std::string& rpc_name) {
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    int param = 0;
    for (; field != attribute->get_field_list().end(); field++, param++) {
        lang_gen_field_unmarshal_impl(*field, source, param);
    }
    source << "    return " << rpc_name << "(";
    for (int i = 0; i < param; i++) {
        if (i + 1 < param) {
            source << "p" << i << ", ";
        } else {
            source << "p" << i;
        }
    }
    source << ");\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl(krpc_field_t* field, krpc_ostream_t& source, int index) {
    if (field->check_array()) {
        lang_gen_field_unmarshal_impl_array(field, source, index);
    } else {
        lang_gen_field_unmarshal_impl_not_array(field, source, index);
    }
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_array(krpc_field_t* field, krpc_ostream_t& source, int index) {
    source << "    " << "std::vector<" << lang_field_find_type_name(field) << "> p" << index  << ";\n";
    source << "    " << "do {\n";
    source << "        " << "krpc_object_t* v = krpc_vector_get(o, " << index << ");\n";
    source << "        " << "for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {\n";
    lang_gen_field_unmarshal_impl_not_array_inline(field, source, index);
    source << "        " << "}\n";
    source << "    " << "} while(0);\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_inline_get(krpc_ostream_t& source, int index, const std::string& method_name) {
    source << "p" << index << ".push_back(" << method_name << "(krpc_vector_get(v, i)));\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_inline(krpc_field_t* field, krpc_ostream_t& source, int index) {
    source << "            ";
    if (field->check_type(krpc_field_type_attribute)) {
        source << lang_field_find_type_name(field) << " o_;\n";
        source << "            " << "unmarshal(krpc_vector_get(v, i), o_);\n";
        source << "            " << "p" << index << ".push_back(o_);\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index, "krpc_string_get");
    }
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_get(krpc_ostream_t& source, int index, const std::string& method_name) {
    source << "    p" << index << " = " << method_name << "(krpc_vector_get(o, " << index << "));\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array(krpc_field_t* field, krpc_ostream_t& source, int index) {
    source << "    " << lang_field_find_type_name(field) << " p" << index  << ";\n";
    if (field->check_type(krpc_field_type_attribute)) {
        source << "    " << "unmarshal(krpc_vector_get(o, " << index << "), p" << index << ");\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index, "krpc_string_get");
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_decls(krpc_ostream_t& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_marshal_method_decl(object->second, header);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_marshal_method_impl(object->second, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_method_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_unmarshal_method_impl(object->second, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_decl(krpc_attribute_t* attribute, krpc_ostream_t& header) {
    header << "krpc_object_t* marshal(" << attribute->get_name() << "& o);\n";
    header << "bool unmarshal(krpc_object_t* v, " << attribute->get_name() << "& o);\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_impl(krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source << "krpc_object_t* marshal(" << attribute->get_name() << "& o) {\n";
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    source << "    " << "krpc_object_t* v = krpc_object_create();\n";
    for (int i = 0; field != attribute->get_field_list().end(); field++, i++) {
        if ((*field)->check_array()) {
            lang_gen_field_marshal_impl_array(*field, source, false);
        } else {
            std::string holder = "o.";
            holder += (*field)->get_field_name();
            lang_gen_field_marshal_impl_not_array(*field, source, holder, "v");
        }
    }
    source << "    return v;\n"
           << "}\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_method_impl(krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source << "bool unmarshal(krpc_object_t* v, " << attribute->get_name() << "& o) {\n";
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    for (int i = 0; field != attribute->get_field_list().end(); field++, i++) {
        std::string name = "o.";
        name += (*field)->get_field_name();
        if ((*field)->check_array()) {
            lang_gen_attribute_unmarshal_field_array(*field, source, name, i);
        } else {            
            lang_gen_attribute_unmarshal_field_not_array(*field, source, name, i);
        }
    }
    source << "    return true;\n"
           << "}\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_array_element(krpc_ostream_t& source, const std::string& name, const std::string& method_name) {
    source << "            " << name << ".push_back(" << method_name << "(krpc_vector_get(v_, i)));\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_array(krpc_field_t* field, krpc_ostream_t& source, const std::string& name, int index) {
    source << "    do {\n"
           << "        krpc_object_t* v_ = 0;\n"
           << "        v_ = krpc_vector_get(v, " << index << ");\n"
           << "        for (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {\n";
    if (field->check_type(krpc_field_type_attribute)) {
        source.write("            @file_type@ o_;\n"
                     "            unmarshal(krpc_vector_get(v_, i), o_);\n"
                     "            @name@.push_back(o_);\n",
                     field->get_field_type().c_str(), name.c_str());
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name, "krpc_string_get");
    }
    source << "        }\n";
    source << "    } while(0);\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_not_array_get(krpc_ostream_t& source, const std::string& name, const std::string& method_name, int index) {
    source << "    " << name << " = " << method_name << "(krpc_vector_get(v, " << index << "));\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_not_array(krpc_field_t* field, krpc_ostream_t& source, const std::string& name, int index) {
    if (field->check_type(krpc_field_type_attribute)) {
        source << "    " << "unmarshal(krpv_vector_get(v, " << index << "), " << name << ");\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_i8", index);
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_i16", index);
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_i32", index);
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_i64", index);
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_ui8", index);
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_ui16", index);
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_ui32", index);
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_ui64", index);
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_f32", index);
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_number_get_f64", index);
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name, "krpc_string_get", index);
    }
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_array(krpc_field_t* field, krpc_ostream_t& source, bool param, const std::string& whites) {
    source << whites << "    do {\n";
    if (!param) {
        source << whites << "        std::vector<" << lang_field_find_type_name(field)  << ">::iterator guard = o." << field->get_field_name() << ".begin();\n";
    } else {
        source << whites << "        std::vector<" << lang_field_find_type_name(field)  << ">::iterator guard = " << field->get_field_name() << ".begin();\n";
    }
    source << whites << "        krpc_object_t* v_ = krpc_object_create();\n"
           << whites << "        krpc_vector_clear(v_);\n";
    if (!param) {
        source << whites << "        " << "for(; guard != o." << field->get_field_name() << ".end(); guard++) {\n";
    } else {
        source << whites << "        " << "for(; guard != " << field->get_field_name() << ".end(); guard++) {\n";
    }
    lang_gen_field_marshal_impl_not_array(field, source, "*guard", "v_", whites + "        ");
    source << whites << "        }\n"
           << whites << "        krpc_vector_push_back(v, v_);\n"
           << whites << "    } while(0);\n";
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_not_array_set(krpc_field_t* field, krpc_ostream_t& source, const std::string& holder, const std::string& v,
    const std::string& method_name, const std::string& suffix, const std::string& whites) {
    source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_" << suffix << " = krpc_object_create();\n"
           << whites << "    " << method_name << "(" << field->get_field_name() << "_" << suffix << ", " << holder << ");\n"
           << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_" << suffix << ");\n";
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_not_array(krpc_field_t* field, krpc_ostream_t& source, const std::string& holder_name,
    const std::string& vector_name, const std::string& whites) {
    std::string holder = field->get_field_name();
    std::string v = "v";
    if (!holder_name.empty()) {
        holder = holder_name;
    }
    if (!vector_name.empty()) {
        v = vector_name;
    }
    if (field->check_type(krpc_field_type_attribute)) {
        source << whites << "    " << "krpc_vector_push_back(" << v << ", marshal(" << holder << "));\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_i8", "i8", whites);
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_i16", "i16", whites);
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_i32", "i32", whites);
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_i64", "i64", whites);
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_ui8", "ui8", whites);
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_ui16", "ui16", whites);
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_ui32", "ui32", whites);
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_ui64", "ui64", whites);
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_f32", "f32", whites);
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v, "krpc_number_set_f64", "f64", whites);
    } else if (field->check_type(krpc_field_type_string)) {
        source << whites << "    krpc_object_t* " << field->get_field_name() << "_string = krpc_object_create();\n"
               << whites << "    krpc_string_set(" << field->get_field_name() << "_string, " << holder << ".c_str());\n"
               << whites << "    krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_string);\n";
    }
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl(krpc_field_t* field, krpc_ostream_t& source, bool param) {
    if (field->check_array()) {
        lang_gen_field_marshal_impl_array(field, source, param);
    } else {
        lang_gen_field_marshal_impl_not_array(field, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attributes_pre_decls(krpc_ostream_t& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        header << "struct " << object->first << ";\n";
    }
    header << "\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_method_impl(krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source.replace("@name@::@name@() {}\n\n"
                   "@name@::@name@(const @name@& rht) {\n",
                   attribute->get_name());
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source.replace("    push_back_all(@name@, rht.@name@.begin(), rht.@name@.end());\n", (*field)->get_field_name());
        } else {
            source.replace("    @name@ = rht.@name@;\n", (*field)->get_field_name());
        }
    }
    source << "}\n\n";
    // operator=()
    source.replace("const @name@& @name@::operator=(const @name@& rht) {\n", attribute->get_name());
    field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source.replace("    push_back_all(@name@, rht.@name@.begin(), rht.@name@.end());\n", (*field)->get_field_name());
        } else {
            source.replace("    @name@ = rht.@name@;\n", (*field)->get_field_name());
        }
    }
    source << "    return *this;\n"
           << "}\n\n";
    // print()
    source.replace("void @name@::print(std::stringstream& ss, std::string white) {\n"
                   "    ss << white << \"@name@:\" << std::endl;\n"
                   "    white += \"  \";\n", attribute->get_name());
    field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            if ((*field)->check_type(krpc_field_type_attribute)) {
                source.replace("    ss << white << \"@name@[\" << std::endl;\n"
                               "    for (size_t i = 0; i < @name@.size(); i++) {\n"
                               "        @name@[i].print(ss, white + \"  \");\n"
                               "    }\n"
                               "    ss << white << \"]\" << std::endl;\n",
                               (*field)->get_field_name());
            } else {
                source.replace("    ss << white << \"@name@[\" << std::endl;\n"
                               "    for (size_t i = 0; i < @name@.size(); i++) {\n", (*field)->get_field_name());
                if (!(*field)->check_type(krpc_field_type_string) && !(*field)->check_type(krpc_field_type_f32) && !(*field)->check_type(krpc_field_type_f64)) {
                    source.replace("        ss << white << \"  @name@=\" << (uint64_t)@name@[i] << std::endl;\n", (*field)->get_field_name());
                } else {
                    source.replace("        ss << white << \"  @name@=\"@name@[i] << std::endl;\n", (*field)->get_field_name());
                }
                source << "    }\n"
                       << "    ss << white << \"]\" << std::endl;\n"; 
            }
        } else {
            if ((*field)->check_type(krpc_field_type_attribute)) {
               source << "    " << (*field)->get_field_name() << ".print(ss, white);\n";
            } else {
                if (!(*field)->check_type(krpc_field_type_string) && !(*field)->check_type(krpc_field_type_f32) && !(*field)->check_type(krpc_field_type_f64)) {
                    source << "    ss << white << \"" << (*field)->get_field_name() << "=\" << (uint64_t)" << (*field)->get_field_name() << " << std::endl;\n";
                } else {
                    source << "    ss << white << \"" << (*field)->get_field_name() << "=\" << " << (*field)->get_field_name() << " << std::endl;\n";
                }
            }
        }
    }
    source << "}\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_method_decl(krpc_attribute_t* attribute, krpc_ostream_t& header) {
    header.replace("    @name@();\n"
                   "    @name@(const @name@& rht);\n"
                   "    const @name@& operator=(const @name@& rht);\n"
                   "    void print(std::stringstream& ss, std::string white = \"\");\n",
                   attribute->get_name());
}

void krpc_gen_cpp_t::lang_gen_attributes(krpc_ostream_t& header, krpc_ostream_t& source) {
    source.replace("#include <sstream>\n"
                   "#include \"@name@.h\"\n\n"
                   "template<typename T>\n"
                   "void push_back_all(std::vector<T>& v, typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end) {\n"
                   "    for (; begin != end; begin++) {\n"
                   "        v.push_back(*begin);\n"
                   "    }\n"
                   "}\n\n",
                   _rpc_gen->get_options()["name"]);
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        header << "struct " << object->first << " {\n";
        krpc_attribute_t::field_list_t::iterator field = object->second->get_field_list().begin();
        for (; field != object->second->get_field_list().end(); field++) {
            lang_gen_attribute_field_decl(header, *field);
        }
        lang_gen_attribute_method_decl(object->second, header);
        lang_gen_attribute_method_impl(object->second, source);
        header << "};\n\n";
    }
    lang_gen_attribute_marshal_method_decls(header);
}

void krpc_gen_cpp_t::lang_gen_attribute_field_decl(krpc_ostream_t& header, krpc_field_t* field) {
    header << "    ";
    if (field->check_array()) {
        // 生成数组类型
        header << "std::vector<" << lang_field_find_type_name(field) << ">";
    } else {
        // 普通变量
        header << lang_field_find_type_name(field);
    }
    header << " " << field->get_field_name() << ";\n";
}

void krpc_gen_cpp_t::lang_gen_rpc_call_param_decl(krpc_ostream_t& header, krpc_field_t* field) {
    if (field->check_array()) {
        // 生成数组类型引用
        header << "std::vector<" << lang_field_find_type_name(field) << ">&";
    } else {
        // 普通变量
        header << lang_param_find_type_name(field);
    }
    header << " " << field->get_field_name();
}

std::string krpc_gen_cpp_t::lang_field_find_type_name(krpc_field_t* field) {
    if (field->check_type(krpc_field_type_i8)) {
        return "int8_t";
    } else if (field->check_type(krpc_field_type_i16)) {
        return "int16_t";
    } else if (field->check_type(krpc_field_type_i32)) {
        return "int32_t";
    } else if (field->check_type(krpc_field_type_i64)) {
        return "int64_t";
    } else if (field->check_type(krpc_field_type_ui8)) {
        return "uint8_t";
    } else if (field->check_type(krpc_field_type_ui16)) {
        return "uint16_t";
    } else if (field->check_type(krpc_field_type_ui32)) {
        return "uint32_t";
    } else if (field->check_type(krpc_field_type_ui64)) {
        return "uint64_t";
    } else if (field->check_type(krpc_field_type_f32)) {
        return "float";
    } else if (field->check_type(krpc_field_type_f64)) {
        return "double";
    } else if (field->check_type(krpc_field_type_string)) {
        return "std::string";
    }
    return field->get_field_type().c_str();
}

std::string krpc_gen_cpp_t::lang_param_find_type_name(krpc_field_t* field) {
    if (field->check_type(krpc_field_type_i8)) {
        return "int8_t";
    } else if (field->check_type(krpc_field_type_i16)) {
        return "int16_t";
    } else if (field->check_type(krpc_field_type_i32)) {
        return "int32_t";
    } else if (field->check_type(krpc_field_type_i64)) {
        return "int64_t";
    } else if (field->check_type(krpc_field_type_ui8)) {
        return "uint8_t";
    } else if (field->check_type(krpc_field_type_ui16)) {
        return "uint16_t";
    } else if (field->check_type(krpc_field_type_ui32)) {
        return "uint32_t";
    } else if (field->check_type(krpc_field_type_ui64)) {
        return "uint64_t";
    } else if (field->check_type(krpc_field_type_f32)) {
        return "float";
    } else if (field->check_type(krpc_field_type_f64)) {
        return "double";
    } else if (field->check_type(krpc_field_type_string)) {
        return "const std::string&";
    }
    return field->get_field_type() + "&";
}
