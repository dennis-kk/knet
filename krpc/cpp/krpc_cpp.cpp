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
    std::stringstream header;
    std::stringstream source;

    header << "//" << std::endl << "// Generated code, DO NOT modify" << std::endl << "//" << std::endl << std::endl;
    source << "//" << std::endl << "// Generated code, DO NOT modify" << std::endl << "//" << std::endl << std::endl;

    header << "#ifndef _krpc_" << options["name"] << "_h_" << std::endl;
    header << "#define _krpc_" << options["name"] << "_h_" << std::endl << std::endl;
    lang_gen_includes(header); // 头文件
    lang_gen_attributes_pre_decls(header); // 预先声明
    lang_gen_attributes(header, source); // 属性对象
    lang_gen_rpc_call_decls(header); // RPC声明
    lang_gen_rpc_call_impls(source); // RPC定义
    lang_gen_attribute_marshal_method_impls(source); // marshal方法
    lang_gen_attribute_unmarshal_method_impls(source); // unmarshal方法
    lang_gen_framework(header, source); // 入口框架
    header << "#endif // _krpc_" << options["name"] << "_h_" << std::endl;
    // 写入.h
    std::ofstream ofs_header;
    std::string header_file_name = options["dir"];
    header_file_name += options["name"];
    header_file_name += ".h";
    ofs_header.open(header_file_name.c_str());
    if (!ofs_header) {
        raise_exception("open file '" << header_file_name << "' failed");
    }
    ofs_header << header.str();
    // 写入.cpp
    std::ofstream ofs_source;
    std::string source_file_name = options["dir"];
    source_file_name += options["name"];
    source_file_name += ".cpp";
    ofs_source.open(source_file_name.c_str());
    if (!ofs_source) {
        raise_exception("open file '" << source_file_name << "' failed");
    }
    ofs_source << source.str();
}

void krpc_gen_cpp_t::lang_gen_includes(std::stringstream& header) {
    header << "#include <cstdint>" << std::endl
           << "#include <string>" << std::endl 
           << "#include <vector>" << std::endl
           << "#include \"knet.h\"" << std::endl << std::endl;
}

void krpc_gen_cpp_t::lang_gen_framework(std::stringstream& header, std::stringstream& source) {
    lang_gen_framework_decls(header);
    lang_gen_framework_impls(source);
}

void krpc_gen_cpp_t::lang_gen_framework_decls(std::stringstream& header) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    header << "class " << options["name"] << "_t {" << std::endl;
    header << "public:" << std::endl;
    header << "    ~" <<  options["name"] << "_t();" << std::endl;
    header << "    static " << options["name"] << "_t* instance();" << std::endl;
    header << "    static void finalize();" << std::endl;
    header << "    int" << " rpc_proc(stream_t* stream);" << std::endl;
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
        header << ");" << std::endl;
    }
    // 私有成员
    header << "private:" << std::endl;
    header << "    " << options["name"] << "_t();" << std::endl;
    header << "    " << options["name"] << "_t(const " << options["name"] << "_t&);" << std::endl;
    header << "    static " << options["name"] << "_t* _instance;" << std::endl;
    header << "    krpc_t* _rpc;" << std::endl;
    header << "};" << std::endl << std::endl;
}

void krpc_gen_cpp_t::lang_gen_framework_impls(std::stringstream& source) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    source << options["name"] << "_t* " << options["name"] << "_t::_instance = 0;" << std::endl << std::endl;
    // 构造函数
    source << options["name"] << "_t::" << options["name"] << "_t() {" << std::endl;
    source << "    _rpc = krpc_create();" << std::endl;
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call = parser->get_rpc_calls().begin();
    // 注册stub
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++, _rpc_id++) {
        source << "    " << "krpc_add_cb(_rpc, " << _rpc_id << ", " << rpc_call->first << "_stub);" << std::endl;
    }
    source << "}" << std::endl << std::endl;
    // 析构函数
    source << options["name"] << "_t::~" << options["name"] << "_t() {" << std::endl;
    source << "    krpc_destroy(_rpc);" << std::endl;
    source << "}" << std::endl << std::endl;
    // 单件指针
    source << options["name"] << "_t* "<< options["name"] << "_t::instance() {" << std::endl;
    source << "    if (!_instance) {" << std::endl <<  "        _instance = new " << options["name"] << "_t();" << std::endl << "    }" << std::endl;
    source << "    return _instance;" << std::endl;
    source << "}" << std::endl << std::endl;
    // 销毁单件
    source << "void " << options["name"] << "_t::" << "finalize() {" << std::endl;
    source << "    if (_instance) {" << std::endl <<  "        delete _instance;" << std::endl << "    }" << std::endl;
    source << "}" << std::endl << std::endl;
    // 处理函数
    source << "int " << options["name"] << "_t::" << "rpc_proc(stream_t* stream) {" << std::endl;
    source << "    return krpc_proc(_rpc, stream);" << std::endl;
    source << "}" << std::endl << std::endl;
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
        source << ") {" << std::endl;
        source << "    return krpc_call(_rpc, stream, " << rpcid << ", " << rpc_call->first << "_proxy(";
        field = rpc_call->second->get_attribute()->get_field_list().begin();
        size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            source << (*field)->get_field_name();
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << "));" << std::endl;
        source << "}" << std::endl << std::endl;
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_decls(std::stringstream& header) {
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
        header << ");" << std::endl;
        header << "int " << rpc_call->first << "_stub(krpc_object_t* o);" << std::endl;
        header << "int " << rpc_call->first << "(";
        field = rpc_call->second->get_attribute()->get_field_list().begin();
        size = rpc_call->second->get_attribute()->get_field_list().size();
        for (size_t pos = 0; field != rpc_call->second->get_attribute()->get_field_list().end(); field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header << ");" << std::endl;
    }
    header << std::endl;
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impls(std::stringstream& source) {
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
        source << ") {" << std::endl;
        lang_gen_rpc_call_impl_proxy(rpc_call->second->get_attribute(), source);
        source << "    return v;" << std::endl;
        source << "}" << std::endl << std::endl;

        source << "int " << rpc_call->first << "_stub(krpc_object_t* o) {" << std::endl;
        lang_gen_rpc_call_impl_stub(rpc_call->second->get_attribute(), source, rpc_call->first);
        source << "}" << std::endl << std::endl;
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impl_proxy(krpc_attribute_t* attribute, std::stringstream& source) {
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    if (size > 0) {
        source << "    " << "krpc_object_t* v = krpc_object_create();" << std::endl;
    }
    for (; field != attribute->get_field_list().end(); field++) {
        lang_gen_field_marshal_impl(*field, source, true);
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impl_stub(krpc_attribute_t* attribute, std::stringstream& source, const std::string& rpc_name) {
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
    source << ");" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl(krpc_field_t* field, std::stringstream& source, int index) {
    if (field->check_array()) {
        lang_gen_field_unmarshal_impl_array(field, source, index);
    } else {
        lang_gen_field_unmarshal_impl_not_array(field, source, index);
    }
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_array(krpc_field_t* field, std::stringstream& source, int index) {
    source << "    " << "std::vector<" << lang_field_find_type_name(field) << "> p" << index  << ";" << std::endl;
    source << "    " << "do {" << std::endl;
    source << "        " << "krpc_object_t* v = krpc_vector_get(o, " << index << ");" << std::endl;
    source << "        " << "for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {" << std::endl;
    lang_gen_field_unmarshal_impl_not_array_inline(field, source, index);
    source << "        " << "}" << std::endl;
    source << "    " << "} while(0);" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_inline(krpc_field_t* field, std::stringstream& source, int index) {
    source << "            ";
    if (field->check_type(krpc_field_type_attribute)) {
        source << lang_field_find_type_name(field) << " __o;" << std::endl;
        source << "            " << "unmarshal(krpc_vector_get(v, i), __o);" << std::endl;
        source << "            " << "p" << index << ".push_back(__o);" << std::endl;
    } else if (field->check_type(krpc_field_type_i8)) {
        source << "p" << index << ".push_back(krpc_number_get_i8(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_i16)) {
        source << "p" << index << ".push_back(krpc_number_get_i16(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_i32)) {
        source << "p" << index << ".push_back(krpc_number_get_i32(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_i64)) {
        source << "p" << index << ".push_back(krpc_number_get_i64(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui8)) {
        source << "p" << index << ".push_back(krpc_number_get_ui8(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui16)) {
        source << "p" << index << ".push_back(krpc_number_get_ui16(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui32)) {
        source << "p" << index << ".push_back(krpc_number_get_ui32(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui64)) {
        source << "p" << index << ".push_back(krpc_number_get_ui64(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_f32)) {
        source << "p" << index << ".push_back(krpc_number_get_f32(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_f64)) {
        source << "p" << index << ".push_back(krpc_number_get_f64(krpc_vector_get(v, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_string)) {
        source << "p" << index << ".push_back(krpc_string_get(krpc_vector_get(v, i)));" << std::endl;
    }
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array(krpc_field_t* field, std::stringstream& source, int index) {
    source << "    " << lang_field_find_type_name(field) << " p" << index  << ";" << std::endl;
    if (field->check_type(krpc_field_type_attribute)) {
        source << "    " << "unmarshal(krpc_vector_get(o, " << index << "), p" << index << ");" << std::endl;
    } else if (field->check_type(krpc_field_type_i8)) {
        source << "    p" << index << " = " << "krpc_number_get_i8(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i16)) {
        source << "    p" << index << " = " << "krpc_number_get_i16(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i32)) {
        source << "    p" << index << " = " << "krpc_number_get_i32(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i64)) {
        source << "    p" << index << " = " << "krpc_number_get_i64(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui8)) {
        source << "    p" << index << " = " << "krpc_number_get_ui8(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui16)) {
        source << "    p" << index << " = " << "krpc_number_get_ui16(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui32)) {
        source << "    p" << index << " = " << "krpc_number_get_ui32(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui64)) {
        source << "    p" << index << " = " << "krpc_number_get_ui64(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_f32)) {
        source << "    p" << index << " = " << "krpc_number_get_f32(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_f64)) {
        source << "    p" << index << " = " << "krpc_number_get_f64(krpc_vector_get(o, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_string)) {
        source << "    p" << index << " = " << "krpc_string_get(krpc_vector_get(o, " << index << "));" << std::endl;
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_decls(std::stringstream& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_marshal_method_decl(object->second, header);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_impls(std::stringstream& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_marshal_method_impl(object->second, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_method_impls(std::stringstream& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_unmarshal_method_impl(object->second, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_decl(krpc_attribute_t* attribute, std::stringstream& header) {
    header << "krpc_object_t* marshal(" << attribute->get_name() << "& o);" << std::endl;
    header << "bool unmarshal(krpc_object_t* v, " << attribute->get_name() << "& o);" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_impl(krpc_attribute_t* attribute, std::stringstream& source) {
    source << "krpc_object_t* marshal(" << attribute->get_name() << "& o) {" << std::endl;
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    source << "    " << "krpc_object_t* v = krpc_object_create();" << std::endl;
    for (int i = 0; field != attribute->get_field_list().end(); field++, i++) {
        if ((*field)->check_array()) {
            lang_gen_field_marshal_impl_array(*field, source, false);
        } else {
            std::string holder = "o.";
            holder += (*field)->get_field_name();
            lang_gen_field_marshal_impl_not_array(*field, source, holder, "v");
        }
    }
    source << "    return v;" << std::endl;
    source << "}" << std::endl << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_method_impl(krpc_attribute_t* attribute, std::stringstream& source) {
    source << "bool unmarshal(krpc_object_t* v, " << attribute->get_name() << "& o) {" << std::endl;
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
    source << "    return true;" << std::endl;
    source << "}" << std::endl << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_array(krpc_field_t* field, std::stringstream& source, const std::string& name, int index) {
    source << "    do {" << std::endl;
    source << "        " << "krpc_object_t* v1 = 0;" << std::endl;
    source << "        " << "v1 = krpc_vector_get(v, " << index << ");" << std::endl;
    source << "        for (uint32_t i = 0; i < krpc_vector_get_size(v1); i++) {" << std::endl;
    if (field->check_type(krpc_field_type_attribute)) {
        source << "            " << field->get_field_type() << " o1;" << std::endl;
        source << "            " << "unmarshal(krpc_vector_get(v1, i), o1);" << std::endl;
        source << "            " << name << ".push_back(o1);" << std::endl;
    } else if (field->check_type(krpc_field_type_i8)) {
        source << "            " << name << ".push_back(krpc_number_get_i8(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_i16)) {
        source << "            " << name << ".push_back(krpc_number_get_i16(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_i32)) {
        source << "            " << name << ".push_back(krpc_number_get_i32(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_i64)) {
        source << "            " << name << ".push_back(krpc_number_get_i64(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui8)) {
        source << "            " << name << ".push_back(krpc_number_get_ui8(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui16)) {
        source << "            " << name << ".push_back(krpc_number_get_ui16(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui32)) {
        source << "            " << name << ".push_back(krpc_number_get_ui32(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui64)) {
        source << "            " << name << ".push_back(krpc_number_get_ui64(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_f32)) {
        source << "            " << name << ".push_back(krpc_number_get_f32(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_f64)) {
        source << "            " << name << ".push_back(krpc_number_get_f64(krpc_vector_get(v1, i)));" << std::endl;
    } else if (field->check_type(krpc_field_type_string)) {
        source << "            " << name << ".push_back(krpc_string_get(krpc_vector_get(v1, i)));" << std::endl;
    }
    source << "        }" << std::endl;
    source << "    } while(0);" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_not_array(krpc_field_t* field, std::stringstream& source, const std::string& name, int index) {
    if (field->check_type(krpc_field_type_attribute)) {
        source << "    " << "unmarshal(krpv_vector_get(v, " << index << "), " << name << ");" << std::endl;
    } else if (field->check_type(krpc_field_type_i8)) {
        source << "    " << name << " = krpc_number_get_i8(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i16)) {
        source << "    " << name << " = krpc_number_get_i16(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i32)) {
        source << "    " << name << " = krpc_number_get_i32(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i64)) {
        source << "    " << name << " = krpc_number_get_i64(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui8)) {
        source << "    " << name << " = krpc_number_get_ui8(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui16)) {
        source << "    " << name << " = krpc_number_get_ui16(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui32)) {
        source << "    " << name << " = krpc_number_get_ui32(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_ui64)) {
        source << "    " << name << " = krpc_number_get_ui64(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_f32)) {
        source << "    " << name << " = krpc_number_get_f32(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_f64)) {
        source << "    " << name << " = krpc_number_get_f64(krpc_vector_get(v, " << index << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_string)) {
        source << "    " << name << " = krpc_string_get(krpc_vector_get(v, " << index << "));" << std::endl;
    }
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_array(krpc_field_t* field, std::stringstream& source, bool param, const std::string& whites) {
    source << whites << "    " << "do {" << std::endl;
    if (!param) {
        source << whites << "        std::vector<" << lang_field_find_type_name(field)  << ">::iterator guard = o." << field->get_field_name() << ".begin();" << std::endl;
    } else {
        source << whites << "        std::vector<" << lang_field_find_type_name(field)  << ">::iterator guard = " << field->get_field_name() << ".begin();" << std::endl;
    }
    source << whites << "        krpc_object_t* v1 = krpc_object_create();" << std::endl;
    source << whites << "        krpc_vector_clear(v1);" << std::endl;
    if (!param) {
        source << whites << "        " << "for(; guard != o." << field->get_field_name() << ".end(); guard++) {" << std::endl;
    } else {
        source << whites << "        " << "for(; guard != " << field->get_field_name() << ".end(); guard++) {" << std::endl;
    }
    lang_gen_field_marshal_impl_not_array(field, source, "*guard", "v1", whites + "        ");
    source << whites << "        " << "}" << std::endl;
    source << whites << "        " << "krpc_vector_push_back(v, v1);" << std::endl;
    source << whites << "    " << "} while(0);" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_not_array(krpc_field_t* field, std::stringstream& source, const std::string& holder_name,
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
        source << whites << "    " << "krpc_vector_push_back(" << v << ", marshal(" << holder << "));" << std::endl;
    } else if (field->check_type(krpc_field_type_i8)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_i8 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_i8(" << field->get_field_name() << "_i8, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_i8);" << std::endl;
    } else if (field->check_type(krpc_field_type_i16)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_i16 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_i16(" << field->get_field_name() << "_i16, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_i16);" << std::endl;
    } else if (field->check_type(krpc_field_type_i32)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_i32 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_i32(" << field->get_field_name() << "_i32, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_i32);" << std::endl;
    } else if (field->check_type(krpc_field_type_i64)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_i64 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_i64(" << field->get_field_name() << "_i64, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_i64);" << std::endl;
    } else if (field->check_type(krpc_field_type_ui8)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_ui8 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_ui8(" << field->get_field_name() << "_ui8, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_ui8);" << std::endl;
    } else if (field->check_type(krpc_field_type_ui16)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_ui16 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_ui16(" << field->get_field_name() << "_ui16, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_ui16);" << std::endl;
    } else if (field->check_type(krpc_field_type_ui32)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_ui32 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_ui32(" << field->get_field_name() << "_ui32, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_ui32);" << std::endl;
    } else if (field->check_type(krpc_field_type_ui64)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_ui64 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_ui64(" << field->get_field_name() << "_ui64, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_ui64);" << std::endl;
    } else if (field->check_type(krpc_field_type_f32)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_f32 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_f32(" << field->get_field_name() << "_f32, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_f32);" << std::endl;
    } else if (field->check_type(krpc_field_type_f64)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_f64 = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_number_set_f64(" << field->get_field_name() << "_f64, " << holder << ");" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_f64);" << std::endl;
    } else if (field->check_type(krpc_field_type_string)) {
        source << whites << "    " << "krpc_object_t* " << field->get_field_name() << "_string = krpc_object_create();" << std::endl;
        source << whites << "    " << "krpc_string_set(" << field->get_field_name() << "_string, " << holder << ".c_str());" << std::endl;
        source << whites << "    " << "krpc_vector_push_back(" << v << ", " << field->get_field_name() << "_string);" << std::endl;
    }
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl(krpc_field_t* field, std::stringstream& source, bool param) {
    if (field->check_array()) {
        lang_gen_field_marshal_impl_array(field, source, param);
    } else {
        lang_gen_field_marshal_impl_not_array(field, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attributes_pre_decls(std::stringstream& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        header << "struct " << object->first << ";" << std::endl;
    }
    header << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attribute_method_impl(krpc_attribute_t* attribute, std::stringstream& source) {
    // ctor
    source << attribute->get_name() << "::" << attribute->get_name() << "() {}" << std::endl << std::endl;
    // copy-ctor
    source << attribute->get_name() << "::" << attribute->get_name() << "(const " << attribute->get_name() << "& rht) {" << std::endl;
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source << "    " << "push_back_all(" << (*field)->get_field_name() << ", " << "rht." << (*field)->get_field_name() <<
                ".begin(), rht." << (*field)->get_field_name() << ".end());" << std::endl;
        } else {
            source << "    " << (*field)->get_field_name() << " = " << "rht." << (*field)->get_field_name() << ";" << std::endl;
        }
    }
    source << "}" << std::endl << std::endl;
    // operator=()
    source << "const " << attribute->get_name() << "& " << attribute->get_name() << "::" << "operator=(const " << attribute->get_name() << "& rht) {" << std::endl;
    field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source << "    " << "push_back_all(" << (*field)->get_field_name() << ", " << "rht." << (*field)->get_field_name() <<
                ".begin(), rht." << (*field)->get_field_name() << ".end());" << std::endl;
        } else {
            source << "    " << (*field)->get_field_name() << " = " << "rht." << (*field)->get_field_name() << ";" << std::endl;
        }
    }
    source << "    " << "return *this;" << std::endl;
    source << "}" << std::endl << std::endl;
    // print()
    source << "void " << attribute->get_name() << "::print(std::stringstream& ss, std::string white) {" << std::endl;
    source << "    ss << white << \"" << attribute->get_name() << ":\" << std::endl;" << std::endl;
    source << "    white += \"  \";" << std::endl;
    field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            if ((*field)->check_type(krpc_field_type_attribute)) {
                source << "    ss << white << \"" << (*field)->get_field_name() << "[\" << std::endl;" << std::endl;
                source << "    for (size_t i = 0; i < " << (*field)->get_field_name() << ".size(); i++) {" << std::endl;
                source << "        " << (*field)->get_field_name() << "[i].print(ss, white + \"  \");" << std::endl;
                source << "    }" << std::endl;
                source << "    ss << white << \"]\" << std::endl;" << std::endl; 
            } else {
                source << "    ss << white << \"" << (*field)->get_field_name() << "[\" << std::endl;" << std::endl;
                source << "    for (size_t i = 0; i < " << (*field)->get_field_name() << ".size(); i++) {" << std::endl;
                if (!(*field)->check_type(krpc_field_type_string) && !(*field)->check_type(krpc_field_type_f32) && !(*field)->check_type(krpc_field_type_f64)) {
                    source << "        ss << white << \"  " << (*field)->get_field_name() << "=\" << (uint64_t)" << (*field)->get_field_name() << "[i] << std::endl;" << std::endl;
                } else {
                    source << "        ss << white << \"  " << (*field)->get_field_name() << "=\" << " << (*field)->get_field_name() << "[i] << std::endl;" << std::endl;
                }
                source << "    }" << std::endl;
                source << "    ss << white << \"]\" << std::endl;" << std::endl; 
            }
        } else {
            if ((*field)->check_type(krpc_field_type_attribute)) {
               source << "    " << (*field)->get_field_name() << ".print(ss, white);" << std::endl;
            } else {
                if (!(*field)->check_type(krpc_field_type_string) && !(*field)->check_type(krpc_field_type_f32) && !(*field)->check_type(krpc_field_type_f64)) {
                    source << "    ss << white << \"" << (*field)->get_field_name() << "=\" << (uint64_t)" << (*field)->get_field_name() << " << std::endl;" << std::endl;
                } else {
                    source << "    ss << white << \"" << (*field)->get_field_name() << "=\" << " << (*field)->get_field_name() << " << std::endl;" << std::endl;
                }
            }
        }
    }
    source << "}" << std::endl << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attribute_method_decl(krpc_attribute_t* attribute, std::stringstream& header) {
    header << "    " << attribute->get_name() << "();" << std::endl;
    header << "    " << attribute->get_name() << "(const " << attribute->get_name() << "& rht);" << std::endl;
    header << "    " << "const " << attribute->get_name() << "& operator=(const " << attribute->get_name() << "& rht);" << std::endl;
    header << "    void print(std::stringstream& ss, std::string white = \"\");" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_attributes(std::stringstream& header, std::stringstream& source) {
    source << "#include <sstream>" << std::endl;
    source << "#include \"" << _rpc_gen->get_options()["name"] << ".h\"" << std::endl << std::endl;
    source << "template<typename T>" << std::endl
           << "void push_back_all(std::vector<T>& v, typename std::vector<T>::const_iterator begin, typename std::vector<T>::const_iterator end) {" << std::endl
           << "    for (; begin != end; begin++) {" << std::endl
           << "        v.push_back(*begin);" << std::endl
           << "    }" << std::endl
           << "}" << std::endl << std::endl;
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object = parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        header << "struct " << object->first << " {" << std::endl;
        krpc_attribute_t::field_list_t::iterator field = object->second->get_field_list().begin();
        for (; field != object->second->get_field_list().end(); field++) {
            lang_gen_attribute_field_decl(header, *field);
        }
        lang_gen_attribute_method_decl(object->second, header);
        lang_gen_attribute_method_impl(object->second, source);
        header << "};" << std::endl << std::endl;
    }
    lang_gen_attribute_marshal_method_decls(header);
}

void krpc_gen_cpp_t::lang_gen_attribute_field_decl(std::stringstream& header, krpc_field_t* field) {
    header << "    ";
    if (field->check_array()) {
        // 生成数组类型
        header << "std::vector<" << lang_field_find_type_name(field) << ">";
    } else {
        // 普通变量
        header << lang_field_find_type_name(field);
    }
    header << " " << field->get_field_name() << ";" << std::endl;
}

void krpc_gen_cpp_t::lang_gen_rpc_call_param_decl(std::stringstream& header, krpc_field_t* field) {
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
