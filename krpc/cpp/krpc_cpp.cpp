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
#include <algorithm>
#include "krpc_cpp.h"
#include "krpc.h"
#include "krpc_parser.h"
#include "krpc_exception.h"

krpc_gen_cpp_t::krpc_gen_cpp_t(krpc_gen_t* rpc_gen)
: _rpc_gen(rpc_gen),
  _parser(_rpc_gen->get_parser()),
  _rpc_id(1) {
}

krpc_gen_cpp_t::~krpc_gen_cpp_t() {
}

void krpc_gen_cpp_t::gen_struct_pre_decls(krpc_ostream_t& header) {
    krpc_parser_t::object_map_t::iterator object =
        _parser->get_attributes().begin();
    for (; object != _parser->get_attributes().end(); object++) {
        header.replace_template("cpp_tpl/header_struct_pre_decl.tpl",
            object->first);
    }
    header << "\n";
}

void krpc_gen_cpp_t::gen_struct_decl(krpc_ostream_t& header, krpc_attribute_t* object) {
    if (!object->get_comment().empty()) {
        header.replace_template("cpp_tpl/header_struct_comment.tpl",
            object->get_comment());
    }
    header << "struct " << object->get_name() << " {\n";
    krpc_attribute_t::field_list_t::iterator field =
        object->get_field_list().begin();
    for (; field != object->get_field_list().end(); field++) {
        gen_struct_field_decl(header, *field);
    }
    gen_struct_method_decl(header, object);
    header << "};\n\n";
}

void krpc_gen_cpp_t::gen_struct_decls(krpc_ostream_t& header) {
    krpc_parser_t::object_map_t::iterator object =
        _parser->get_attributes().begin();
    for (; object != _parser->get_attributes().end(); object++) {
        gen_struct_decl(header, object->second);
    }
}

void krpc_gen_cpp_t::gen_struct_method_decl(krpc_ostream_t& header, krpc_attribute_t* object) {
    header.replace_template("cpp_tpl/header_struct_method_decl.tpl", object->get_name());
}

void krpc_gen_cpp_t::gen_entry_decl(krpc_ostream_t& header) {
    header.replace_template("cpp_tpl/header_entry_class_decl.tpl", _rpc_gen->get_options()["name"]);
    gen_entry_rpc_method_decls(header);
    header.replace_template("cpp_tpl/header_entry_class_private_field.tpl",
        _rpc_gen->get_options()["name"]);
}

void krpc_gen_cpp_t::gen_entry_rpc_method_decls(krpc_ostream_t& header) {
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        _parser->get_rpc_calls().begin();
    for (; rpc_call != _parser->get_rpc_calls().end(); rpc_call++) {
        gen_entry_rpc_method_decl(header, rpc_call->second);
    }
}

void krpc_gen_cpp_t::gen_entry_rpc_method_param_comment(krpc_ostream_t& header, krpc_field_t* field) {
    header.write_template("cpp_tpl/header_struct_field_comment.tpl",
        field->get_field_name().c_str(),
        (field->get_comment().empty() ? "N/A." : field->get_comment().c_str()));
}

void krpc_gen_cpp_t::gen_entry_rpc_method_comment(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call) {
    header.write_template("cpp_tpl/header_entry_class_rpc_method_comment_begin.tpl",
        rpc_call->get_name().c_str(),
        rpc_call->get_comment().empty() ? "代理调用包装方法" : rpc_call->get_comment().c_str());
    krpc_attribute_t* attribute = rpc_call->get_attribute();
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    header << "\t * \\param stream stream_t实例\n";
    for (; field != attribute->get_field_list().end(); field++) {
        gen_entry_rpc_method_param_comment(header, *field);
    }
    header.write_template("cpp_tpl/header_entry_class_rpc_method_comment_end.tpl");
}

void krpc_gen_cpp_t::gen_entry_rpc_method_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call) {
    gen_entry_rpc_method_comment(header, rpc_call);
    header << "\tint " << rpc_call->get_name() << "(stream_t* stream, ";
    krpc_attribute_t* attribute = rpc_call->get_attribute();
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    for (size_t pos = 0; field != attribute->get_field_list().end();
        field++, pos++) {
        gen_rpc_call_param_decl(header, *field);
        if (pos + 1 < size) {
            header << ", ";
        }
    }
    header.write(");\n\n");
}

void krpc_gen_cpp_t::gen_struct_marshal_unmarshal_method_decls(krpc_ostream_t& header) {
    krpc_parser_t::object_map_t::iterator object =
        _parser->get_attributes().begin();
    for (; object != _parser->get_attributes().end(); object++) {        
        gen_struct_marshal_unmarshal_method_decl(header, object->second);
    }
}

void krpc_gen_cpp_t::gen_struct_marshal_unmarshal_method_decl(krpc_ostream_t& header, krpc_attribute_t* object) {
    header.replace_template("cpp_tpl/header_struct_marshal_unmarshal_decl.tpl", object->get_name());
}

void krpc_gen_cpp_t::gen_rpc_call_proxy_decls(krpc_ostream_t& header) {
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        _parser->get_rpc_calls().begin();
    for (; rpc_call != _parser->get_rpc_calls().end(); rpc_call++) {
        gen_rpc_call_proxy_decl(header, rpc_call->second);
    }
}

void krpc_gen_cpp_t::gen_rpc_call_proxy_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call) {
    header.replace_template("cpp_tpl/header_rpc_call_proxy_begin.tpl", rpc_call->get_name());
    krpc_attribute_t* attribute = rpc_call->get_attribute();
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    for (size_t pos = 0; field != attribute->get_field_list().end();
        field++, pos++) {
        gen_rpc_call_param_decl(header, *field);
        if (pos + 1 < size) {
            header << ", ";
        }
    }
    header << ");\n\n";
}

void krpc_gen_cpp_t::gen_rpc_call_stub_decls(krpc_ostream_t& header) {
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        _parser->get_rpc_calls().begin();
    for (; rpc_call != _parser->get_rpc_calls().end(); rpc_call++) {
        gen_rpc_call_stub_decl(header, rpc_call->second);
    }
}

void krpc_gen_cpp_t::gen_rpc_call_stub_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call) {
    header.replace_template("cpp_tpl/header_rpc_call_stub_decl.tpl", rpc_call->get_name());
}

void krpc_gen_cpp_t::gen_rpc_call_decls(krpc_ostream_t& header) {
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        _parser->get_rpc_calls().begin();
    for (; rpc_call != _parser->get_rpc_calls().end(); rpc_call++) {
        gen_rpc_call_decl(header, rpc_call->second);
    }
}

void krpc_gen_cpp_t::gen_rpc_call_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call) {
    header.write(
        "/**\n"
        " * {{@comment}}, {{@method_name}}声明，需实现此方法\n",
        (rpc_call->get_comment().empty() ? "N/A." : rpc_call->get_comment().c_str()),
        rpc_call->get_name().c_str());
    krpc_attribute_t* attribute = rpc_call->get_attribute();
    krpc_attribute_t::field_list_t::iterator field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        header.write(
            " * \\param {{@name}} {{@comment}}\n",
            (*field)->get_field_name().c_str(),
            ((*field)->get_comment().empty() ? "N/A." : (*field)->get_comment().c_str()));
    }
    header.write(
        " * \\retval rpc_ok          成功\n"
        " * \\retval rpc_close       忽略错误，关闭\n"
        " * \\retval rpc_error       错误，但不关闭\n"
        " * \\retval rpc_error_close 错误且关闭\n"
        " */\n"
        "int {{@method_name}}(",
        rpc_call->get_name().c_str());
    field = attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    for (size_t pos = 0; field != attribute->get_field_list().end();
        field++, pos++) {
        gen_rpc_call_param_decl(header, *field);
        if (pos + 1 < size) {
            header << ", ";
        }
    }
    header << ");\n\n";
}

void krpc_gen_cpp_t::gen_header_file() {
    krpc_ostream_t header(_rpc_gen->get_option("dir") + _rpc_gen->get_option("name") + ".h");
    // 头文件前半部分
    header.replace_template("cpp_tpl/header_pre_decls.tpl", _rpc_gen->get_option("name"));
    // struct预先声明
    gen_struct_pre_decls(header);
    // struct声明
    gen_struct_decls(header);
    // struct marshal方法声明
    gen_struct_marshal_unmarshal_method_decls(header);
    // rpc proxy声明
    gen_rpc_call_proxy_decls(header);
    // rpc stub声明
    gen_rpc_call_stub_decls(header);
    // rpc方法声明
    gen_rpc_call_decls(header);
    // 入口类单件声明
    gen_entry_decl(header);
    // 头文件结束
    header.replace_template("cpp_tpl/header_end.tpl", _rpc_gen->get_option("name"));
}

void krpc_gen_cpp_t::gen_entry_impls(krpc_ostream_t& source) {
    // 构造函数
    gen_entry_ctor(source);
    source.replace_template("cpp_tpl/source_entry_class_impls.tpl",
        _rpc_gen->get_options()["name"]);
    // rpc包装方法
    gen_entry_rpc_call_wrapper_impls(source);
}

void krpc_gen_cpp_t::gen_entry_ctor(krpc_ostream_t& source) {
    source.replace_template("cpp_tpl/source_entry_ctor_begin.tpl",
        _rpc_gen->get_options()["name"]);
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        parser->get_rpc_calls().begin();
    // 注册stub
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++, _rpc_id++) {
        source.write_template("cpp_tpl/source_add_cb.tpl",
            _rpc_id, rpc_call->first.c_str());
    }
    source << "}\n\n";
}

void krpc_gen_cpp_t::gen_entry_rpc_call_wrapper_impls(krpc_ostream_t& source) {
    // RPC包装方法
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call = parser->get_rpc_calls().begin();
    for (int rpcid = 1; rpc_call != parser->get_rpc_calls().end();
        rpc_call++, rpcid++) {
        gen_entry_rpc_call_wrapper_impl(source, rpc_call->second, rpcid);
    }
}

void krpc_gen_cpp_t::gen_entry_rpc_call_wrapper_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call, int rpcid) {
    gen_entry_rpc_call_wrapper_method_prototype(source, rpc_call);
    gen_entry_rpc_call_wrapper_method_impl(source, rpc_call, rpcid);
}

void krpc_gen_cpp_t::gen_entry_rpc_call_wrapper_method_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call, int rpcid) {
    source << "\tkrpc_object_t* o = "
           << rpc_call->get_name() << "_proxy(";
    krpc_attribute_t* attribute = rpc_call->get_attribute();
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    for (size_t pos = 0; field != attribute->get_field_list().end();
        field++, pos++) {
        source << (*field)->get_field_name();
        if (pos + 1 < size) {
            source << ", ";
        }
    }
    source.write_template("cpp_tpl/source_entry_rpc_call_wrapper_method_end.tpl", rpcid);
}

void krpc_gen_cpp_t::gen_entry_rpc_call_wrapper_method_prototype(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    source << "int " << options["name"] << "_t::"
            << rpc_call->get_name() << "(stream_t* stream, ";
    krpc_attribute_t* attribute = rpc_call->get_attribute();
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    for (size_t pos = 0; field != attribute->get_field_list().end();
        field++, pos++) {
        gen_rpc_call_param_decl(source, *field);
        if (pos + 1 < size) {
            source << ", ";
        }
    }
    source << ") {\n";
}

void krpc_gen_cpp_t::gen_struct_marshal_method_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        gen_struct_marshal_method_impl(source, object->second);
    }
}

void krpc_gen_cpp_t::gen_struct_marshal_field_impl(krpc_ostream_t& source, krpc_field_t* field) {
    if (field->check_array()) {
        gen_field_marshal_impl_array(field, source, false);
    } else if (field->check_table()) {
        gen_field_marshal_impl_table(field, source, false);
    } else {
        std::string holder = "o." + field->get_field_name();
        gen_field_marshal_impl_not_array(field, source, holder, "v");
    }
}

void krpc_gen_cpp_t::gen_struct_marshal_method_impl(krpc_ostream_t& source, krpc_attribute_t* object) {
    source.replace_template("cpp_tpl/source_struct_marshal_method_begin.tpl",object->get_name());
    krpc_attribute_t::field_list_t::iterator field =
        object->get_field_list().begin();
    for (int i = 0; field != object->get_field_list().end(); field++, i++) {
        gen_struct_marshal_field_impl(source, *field);
    }
    source.write_template("cpp_tpl/source_struct_marshal_method_end.tpl");
}

void krpc_gen_cpp_t::gen_struct_unmarshal_method_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {        
        gen_struct_unmarshal_method_impl(source, object->second);
    }
}

void krpc_gen_cpp_t::gen_struct_unmarshal_method_impl(krpc_ostream_t& source, krpc_attribute_t* object) {
    source.replace_template("cpp_tpl/struct_unmarshal_method_begin.tpl", object->get_name());
    krpc_attribute_t::field_list_t::iterator field =
        object->get_field_list().begin();
    for (int i = 0; field != object->get_field_list().end(); field++, i++) {
        std::string name = "o." + (*field)->get_field_name();
        gen_struct_unmarshal_field_impl(source, *field, name, i);
    }
    source.write_template("cpp_tpl/struct_unmarshal_method_end.tpl");
}

void krpc_gen_cpp_t::gen_struct_unmarshal_field_impl(krpc_ostream_t& source, krpc_field_t* field, const std::string& name, int index) {
    if (field->check_array()) {
        gen_attribute_unmarshal_field_array(field, source, name, index);
    } else if (field->check_table()) {
        gen_struct_unmarshal_field_table(source, field, index);
    } else {            
        gen_attribute_unmarshal_field_not_array(field, source, name, index);
    }
}

void krpc_gen_cpp_t::gen_rpc_call_proxy_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        source << "krpc_object_t* " << rpc_call->first << "_proxy(";
        krpc_attribute_t* attribute = rpc_call->second->get_attribute();
        krpc_attribute_t::field_list_t::iterator field =
            attribute->get_field_list().begin();
        size_t size = attribute->get_field_list().size();
        for (size_t pos = 0; field != attribute->get_field_list().end();
            field++, pos++) {
            gen_rpc_call_param_decl(source, *field);
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << ") {\n";
        gen_rpc_call_proxy_impl(source, rpc_call->second);
        source << "\treturn v;\n"
               << "}\n\n";
    }
}

void krpc_gen_cpp_t::gen_rpc_call_proxy_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call) {
    krpc_attribute_t::field_list_t::iterator field =
        rpc_call->get_attribute()->get_field_list().begin();
    size_t size = rpc_call->get_attribute()->get_field_list().size();
    if (size > 0) {
        source << "\tkrpc_object_t* v = krpc_object_create();\n";
    }
    for (; field != rpc_call->get_attribute()->get_field_list().end(); field++) {
        gen_field_marshal_impl(*field, source, true);
    }
}

void krpc_gen_cpp_t::gen_rpc_call_stub_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        source << "int " << rpc_call->first << "_stub(krpc_object_t* o) {\n";
        gen_rpc_call_stub_impl(source, rpc_call->second);
        source << "}\n\n";
    }
}

void krpc_gen_cpp_t::gen_rpc_call_stub_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call) {
    krpc_attribute_t::field_list_t::iterator field =
        rpc_call->get_attribute()->get_field_list().begin();
    int param = 0;
    for (; field != rpc_call->get_attribute()->get_field_list().end(); field++, param++) {
        gen_field_unmarshal_impl(*field, source, param);
    }
    source << "\treturn " << rpc_call->get_name() << "(";
    for (int i = 0; i < param; i++) {
        if (i + 1 < param) {
            source << "p" << i << ", ";
        } else {
            source << "p" << i;
        }
    }
    source << ");\n";
}

void krpc_gen_cpp_t::gen_struct_method_impls(krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        gen_struct_method_impl(source, object->second);
    }
}

void krpc_gen_cpp_t::gen_struct_method_impl(krpc_ostream_t& source, krpc_attribute_t* object) {
    source.replace_template("cpp_tpl/source_struct_method_begin.tpl", object->get_name());
    krpc_attribute_t::field_list_t::iterator field =
        object->get_field_list().begin();
    for (; field != object->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source.replace_template("cpp_tpl/source_struct_field_assign_array.tpl",
                (*field)->get_field_name());
        } else if ((*field)->check_table()) {
            source.replace_template("cpp_tpl/source_struct_field_assign_table.tpl",
                (*field)->get_field_name());
        } else {
            source.replace_template("cpp_tpl/source_struct_field_assign_common.tpl",
                (*field)->get_field_name());
        }
    }
    source << "}\n\n";
    // operator=()
    source.replace_template("cpp_tpl/source_struct_operator_assign_begin.tpl",
        object->get_name());
    field = object->get_field_list().begin();
    for (; field != object->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source.replace_template("cpp_tpl/source_struct_field_assign_array.tpl",
                (*field)->get_field_name());
        } else if ((*field)->check_table()) {
            source.replace_template("cpp_tpl/source_struct_field_assign_table.tpl",
                (*field)->get_field_name());
        } else {
            source.replace_template("cpp_tpl/source_struct_field_assign_common.tpl",
                (*field)->get_field_name());
        }
    }
    source.write_template("cpp_tpl/source_struct_operator_assign_end.tpl");
    // print()
    gen_attribute_method_print_impl(object, source);
}

void krpc_gen_cpp_t::gen_struct_unmarshal_field_table(krpc_ostream_t& source, krpc_field_t* field, int index) {
    source.write_template("cpp_tpl/source_struct_unmarshal_field_table_begin.tpl", index);
    // 属性unmarshal
    if (field->check_value_type(krpc_field_type_attribute)) {
        source.write_template("cpp_tpl/struct_unmarshal_field_table_object.tpl",
            field->get_value_type_name().c_str(),
            field->get_field_name().c_str(),
            param_find_table_key_get_func_name(field).c_str());
    } else {
        source.write_template("cpp_tpl/struct_unmarshal_field_table_common.tpl",
            field->get_field_name().c_str(),
            param_find_table_key_get_func_name(field).c_str(),
            param_find_table_value_get_func_name(field).c_str());
    }
    source.write_template("cpp_tpl/struct_unmarshal_field_table_end.tpl");
}

void krpc_gen_cpp_t::gen_source_file() {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    krpc_ostream_t source(options["dir"] + options["name"] + ".cpp");
    // 前半部分
    source.replace_template("cpp_tpl/source_pre_decls.tpl", options["name"]);
    // 入口类实现
    gen_entry_impls(source);
    // struct marshal方法实现
    gen_struct_marshal_method_impls(source);
    // struct unmarshal方法实现
    gen_struct_unmarshal_method_impls(source);
    // rpc proxy
    gen_rpc_call_proxy_impls(source);
    // rpc stub
    gen_rpc_call_stub_impls(source);
    // struct 成员函数实现
    gen_struct_method_impls(source);
    // 结束
    source << "} // namespace " << options["name"] << "\n\n";
}

void krpc_gen_cpp_t::gen_code() {
    gen_header_file();
    gen_source_file();
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl(krpc_field_t* field,
    krpc_ostream_t& source, int index) {
    if (field->check_array()) {
        gen_field_unmarshal_impl_array(field, source, index);
    } else if (field->check_table()) {
        gen_field_unmarshal_impl_table(field, source, index);
    } else {
        gen_field_unmarshal_impl_not_array(field, source, index);
    }
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_table(krpc_field_t* field,
    krpc_ostream_t& source, int index) {
    source.write_template("cpp_tpl/source_field_unmarshal_table_begin.tpl",
        field_find_type_name(field).c_str(),
        field_find_value_type_name(field).c_str(),
        index,
        index);
    std::string key_get_func = param_find_table_key_get_func_name(field);
    std::string value_get_func = param_find_table_value_get_func_name(field);
    if (field->check_value_type(krpc_field_type_attribute)) {
        source.write_template("cpp_tpl/source_field_unmarshal_table_object.tpl",
            field->get_value_type_name().c_str(),
            index,
            key_get_func.c_str());
    } else {
        source.write_template("cpp_tpl/source_field_unmarshal_table_common.tpl",
            index,
            key_get_func.c_str(),
            value_get_func.c_str());
    }
    source << "\t\t\twhile (krpc_map_next(m_, &k_, &v_)) {\n";
    key_get_func = param_find_table_key_get_func_name(field);
    value_get_func = param_find_table_value_get_func_name(field);
    if (field->check_value_type(krpc_field_type_attribute)) {
        source.write_template("cpp_tpl/source_field_unmarshal_table_object_while.tpl",
            field->get_value_type_name().c_str(),
            index,
            key_get_func.c_str());
    } else {
        source.write_template("cpp_tpl/source_field_unmarshal_table_common_while.tpl",
            index,
            key_get_func.c_str(),
            value_get_func.c_str());
    }
    source.write_template("cpp_tpl/source_field_unmarshal_table_end.tpl");
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_array(krpc_field_t* field,
    krpc_ostream_t& source, int index) {
    source.write_template("cpp_tpl/source_field_unmarshal_array_begin.tpl",
        field_find_type_name(field).c_str(),
        index,
        index);
    gen_field_unmarshal_impl_not_array_inline(field, source, index);
    source.write_template("cpp_tpl/source_field_unmarshal_array_end.tpl");
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_not_array_inline_get(
    krpc_ostream_t& source, int index, const std::string& method_name) {
    source.write_template("cpp_tpl/source_field_unmarshal_impl_not_array_inline_get.tpl",
        index,
        method_name.c_str());
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_not_array_inline_get_common(krpc_field_t* field, krpc_ostream_t& source,
    int index) {
    if (field->check_type(krpc_field_type_i8)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i16)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_string_get");
    }
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_not_array_inline(
    krpc_field_t* field, krpc_ostream_t& source, int index) {
    source << "\t\t\t";
    if (field->check_type(krpc_field_type_attribute)) {
        source.write_template("cpp_tpl/source_field_unmarshal_not_array_inline_attribute.tpl",
            field_find_type_name(field).c_str(),
            index);
    } else {
        gen_field_unmarshal_impl_not_array_inline_get_common(field, source, index);
    }
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_not_array_get(
    krpc_ostream_t& source, int index,
        const std::string& method_name) {
    source.write_template("cpp_tpl/source_field_unmarshal_not_array_get.tpl",
        index,
        method_name.c_str(),
        index);
}

void krpc_gen_cpp_t::gen_field_unmarshal_impl_not_array(
    krpc_field_t* field, krpc_ostream_t& source, int index) {
    source.write_template("cpp_tpl/source_field_unmarshal_not_array_begin.tpl",
        field_find_type_name(field).c_str(),
        index);
    if (field->check_type(krpc_field_type_attribute)) {
        source.write("\tunmarshal(krpc_vector_get(o, {{$index}}), p{{$index}});\n",
            index, index);
    } else if (field->check_type(krpc_field_type_i8)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_string_get");
    }
}

void krpc_gen_cpp_t::gen_attribute_unmarshal_field_array_element(
    krpc_ostream_t& source, const std::string& name,
        const std::string& method_name) {
    source.write("\t\t\t{{@name}}.push_back({{@method_name}}(krpc_vector_get(v_, i)));\n",
        name.c_str(),
        method_name.c_str());
}

void krpc_gen_cpp_t::gen_attribute_unmarshal_field_array(
    krpc_field_t* field, krpc_ostream_t& source, const std::string& name,
        int index) {
    source.write(
        "\tdo {\n"
        "\t\tkrpc_object_t* v_ = 0;\n"
        "\t\tv_ = krpc_vector_get(v, {{$index}});\n"
        "\t\tfor (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {\n",
        index);
    if (field->check_type(krpc_field_type_attribute)) {
        source.write(
            "\t\t\t{{@file_type}} o_;\n"
            "\t\t\tunmarshal(krpc_vector_get(v_, i), o_);\n"
            "\t\t\t{{@name}}.push_back(o_);\n",
            field->get_field_type_name().c_str(), name.c_str());
    } else if (field->check_type(krpc_field_type_i8)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_string_get");
    }
    source.write("\t\t}\n"
                 "\t} while(0);\n");
}

void krpc_gen_cpp_t::gen_attribute_unmarshal_field_not_array_get(
    krpc_ostream_t& source, const std::string& name,
        const std::string& method_name, int index) {
    source.write("\t{{@name}} = {{@method_name}}(krpc_vector_get(v, {{$index}}));\n",
        name.c_str(), method_name.c_str(), index);
}

void krpc_gen_cpp_t::gen_attribute_unmarshal_field_not_array(
    krpc_field_t* field, krpc_ostream_t& source, const std::string& name,
        int index) {
    if (field->check_type(krpc_field_type_attribute)) {
        source.write("\tunmarshal(krpv_vector_get(v, {{$index}}), {{@name}});\n",
            index, name.c_str());
    } else if (field->check_type(krpc_field_type_i8)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i8", index);
    } else if (field->check_type(krpc_field_type_i16)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i16", index);
    } else if (field->check_type(krpc_field_type_i32)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i32", index);
    } else if (field->check_type(krpc_field_type_i64)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i64", index);
    } else if (field->check_type(krpc_field_type_ui8)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui8", index);
    } else if (field->check_type(krpc_field_type_ui16)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui16", index);
    } else if (field->check_type(krpc_field_type_ui32)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui32", index);
    } else if (field->check_type(krpc_field_type_ui64)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui64", index);
    } else if (field->check_type(krpc_field_type_f32)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_f32", index);
    } else if (field->check_type(krpc_field_type_f64)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_f64", index);
    } else if (field->check_type(krpc_field_type_string)) {
        gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_string_get", index);
    }
}

void krpc_gen_cpp_t::gen_field_marshal_impl_array(krpc_field_t* field,
    krpc_ostream_t& source, bool param, const std::string& whites) {
    source << whites << "\tdo {\n";
    if (!param) {
        source.write(
            "{{@whites}}\t\tstd::vector<{{@type}}>::iterator guard = o.{{@name}}.begin();\n",
            whites.c_str(),
            field_find_type_name(field).c_str(),
            field->get_field_name().c_str());
    } else {
        source.write(
            "{{@whites}}\t\tstd::vector<{{@type}}>::iterator guard = {{@name}}.begin();\n",
            whites.c_str(),
            field_find_type_name(field).c_str(),
            field->get_field_name().c_str());
    }
    source.write(
        "{{@whites}}\t\tkrpc_object_t* v_ = krpc_object_create();\n"
        "{{@whites}}\t\tkrpc_vector_clear(v_);\n",
        whites.c_str(),
        whites.c_str());
    if (!param) {
        source.write(
            "{{@whites}}\t\tfor(; guard != o.{{@name}}.end(); guard++) {\n",
            whites.c_str(),
            field->get_field_name().c_str());
    } else {
        source.write(
            "{{@whites}}\t\tfor(; guard != {{@name}}.end(); guard++) {\n",
            whites.c_str(),
            field->get_field_name().c_str());
    }
    gen_field_marshal_impl_not_array(field, source, "(*guard)", "v_",
        whites + "\t\t");
    source.write(
        "{{@whites}}\t\t}\n"
        "{{@whites}}\t\tkrpc_vector_push_back(v, v_);\n"
        "{{@whites}}\t} while(0);\n",
        whites.c_str(),
        whites.c_str(),
        whites.c_str());
}

void krpc_gen_cpp_t::gen_field_marshal_impl_table(krpc_field_t* field,
    krpc_ostream_t& source, bool param, const std::string& whites) {
    source << whites << "\tdo {\n";
    if (!param) {
        source.write(
            "{{@whites}}\t\tstd::map<{{@key}}, {{@value}}>::iterator guard = o.{{@name}}.begin();\n",
            whites.c_str(),
            field_find_type_name(field).c_str(),
            field_find_value_type_name(field).c_str(),
            field->get_field_name().c_str());
    } else {
        source.write(
            "{{@whites}}\t\tstd::map<{{@key}}, {{@value}}>::iterator guard = {{@name}}.begin();\n",
            whites.c_str(),
            field_find_type_name(field).c_str(),
            field_find_value_type_name(field).c_str(),
            field->get_field_name().c_str());
    }
    source.write(
        "\t\tkrpc_object_t* m_ = krpc_object_create();\n"
        "\t\tkrpc_map_clear(m_);\n");
    if (!param) {
        std::string key_get = "guard->first";
        if (field->check_key_type(krpc_field_type_string)) {
            key_get = "guard->first.c_str()";
        }
        source.write(
            "{{@whites}}\t\tfor(; guard != o.{{@name}}.end(); guard++) {\n"
            "\t\t\tkrpc_object_t* k_ = krpc_object_create();\n"
            "\t\t\t{{@set_func}}(k_, {{@key_get}});\n",
               whites.c_str(),
               field->get_field_name().c_str(),
               param_find_table_key_set_func_name(field).c_str(), 
               key_get.c_str());
        std::string func = param_find_table_value_set_func_name(field);
        if (func.empty()) {
            source << "\t\t\tkrpc_object_t* v_ = marshal(guard->second);\n";
        } else {
            std::string value_get = "guard->second";
            if (field->check_value_type(krpc_field_type_string)) {
                value_get = "guard->second.c_str()";
            }
            source.write(
                "\t\t\tkrpc_object_t* v_ = krpc_object_create();\n"
                "\t\t\t{{@set_func}}(v_, {{@value_get}});\n",
                param_find_table_value_set_func_name(field).c_str(),
                value_get.c_str());
        }
        source.write(
            "\t\t\tkrpc_map_insert(m_, k_, v_);\n"
            "{{@whites}}\t\t}\n",
            whites.c_str());
    } else {
        std::string key_get = "guard->first";
        if (field->check_key_type(krpc_field_type_string)) {
            key_get = "guard->first.c_str()";
        }
        source.write(
            "{{@whites}}\t\tfor(; guard != {{@name}}.end(); guard++) {\n"
            "\t\t\tkrpc_object_t* k_ = krpc_object_create();\n"
            "\t\t\t{{@set_func}}(k_, {{@key_get}});\n",
            whites.c_str(),
            field->get_field_name().c_str(),
            param_find_table_key_set_func_name(field).c_str(),
            key_get.c_str());
        std::string func = param_find_table_value_set_func_name(field);
        if (func.empty()) {
            source << "\t\t\tkrpc_object_t* v_ = marshal(guard->second);\n";
        } else {
            std::string value_get = "guard->second";
            if (field->check_value_type(krpc_field_type_string)) {
                value_get = "guard->second.c_str()";
            }
            source.write(
                "\t\t\tkrpc_object_t* v_ = krpc_object_create();\n"
                "\t\t\t{{@set_func}}(v_, {{@value_get}});\n",
                param_find_table_value_set_func_name(field).c_str(),
                value_get.c_str());
            }
        source.write(
            "\t\t\tkrpc_map_insert(m_, k_, v_);\n"
            "{{@whites}}\t\t}\n",
            whites.c_str());
    }
    source.replace(
        "{{@whites}}\t\tkrpc_vector_push_back(v, m_);\n"
        "{{@whites}}\t} while(0);\n",
        whites.c_str());
}

void krpc_gen_cpp_t::gen_field_marshal_impl_not_array_set(
    krpc_field_t* field, krpc_ostream_t& source, const std::string& holder,
        const std::string& v, const std::string& method_name,
            const std::string& suffix, const std::string& whites) {
    source.write(
        "{{@whites}}\tkrpc_object_t* {{@name}}_{{@suffix}} = krpc_object_create();\n"
        "{{@whites}}\t{{@method}}({{@name}}_{{@suffix}}, {{@holder}});\n"
        "{{@whites}}\tkrpc_vector_push_back({{@v}}, {{@name}}_{{@suffix}});\n",
        whites.c_str(),
        field->get_field_name().c_str(),
        suffix.c_str(),
        whites.c_str(),
        method_name.c_str(),
        field->get_field_name().c_str(),
        suffix.c_str(),
        holder.c_str(),
        whites.c_str(),
        v.c_str(),
        field->get_field_name().c_str(),
        suffix.c_str());
}

void krpc_gen_cpp_t::gen_field_marshal_impl_not_array(krpc_field_t* field,
    krpc_ostream_t& source, const std::string& holder_name,
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
        source << whites << "\t" << "krpc_vector_push_back(" << v
               << ", marshal(" << holder << "));\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i8", "i8", whites);
    } else if (field->check_type(krpc_field_type_i16)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i16", "i16", whites);
    } else if (field->check_type(krpc_field_type_i32)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i32", "i32", whites);
    } else if (field->check_type(krpc_field_type_i64)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i64", "i64", whites);
    } else if (field->check_type(krpc_field_type_ui8)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui8", "ui8", whites);
    } else if (field->check_type(krpc_field_type_ui16)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui16", "ui16", whites);
    } else if (field->check_type(krpc_field_type_ui32)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui32", "ui32", whites);
    } else if (field->check_type(krpc_field_type_ui64)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui64", "ui64", whites);
    } else if (field->check_type(krpc_field_type_f32)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_f32", "f32", whites);
    } else if (field->check_type(krpc_field_type_f64)) {
        gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_f64", "f64", whites);
    } else if (field->check_type(krpc_field_type_string)) {        
        source.write(
            "{{@whites}}\tkrpc_object_t* {{@name}}_string = krpc_object_create();\n"
            "{{@whites}}\tkrpc_string_set({{@name}}_string, {{@hold}}.c_str());\n"
            "{{@whites}}\tkrpc_vector_push_back({{@v}}, {{@name}}_string);\n",
            whites.c_str(),
            field->get_field_name().c_str(),
            whites.c_str(),
            field->get_field_name().c_str(),
            holder.c_str(),
            whites.c_str(),
            v.c_str(),
            field->get_field_name().c_str());
    }
}

void krpc_gen_cpp_t::gen_field_marshal_impl(krpc_field_t* field,
    krpc_ostream_t& source, bool param) {
    if (field->check_array()) {
        gen_field_marshal_impl_array(field, source, param);
    } else if (field->check_table()) {
        gen_field_marshal_impl_table(field, source, param);
    } else {
        gen_field_marshal_impl_not_array(field, source);
    }
}

void krpc_gen_cpp_t::gen_attribute_method_print_impl_array(
    krpc_field_t* field, krpc_ostream_t& source) {
    if (field->check_type(krpc_field_type_attribute)) {
        source.replace(
            "\tss << white << \"{{@name}}[\" << std::endl;\n"
            "\tfor (size_t i = 0; i < {{@name}}.size(); i++) {\n"
            "\t\t{{@name}}[i].print(ss, white + \"  \");\n"
            "\t}\n"
            "\tss << white << \"]\" << std::endl;\n",
            field->get_field_name());
    } else {
        source.replace(
            "\tss << white << \"{{@name}}[\" << std::endl;\n"
            "\tfor (size_t i = 0; i < {{@name}}.size(); i++) {\n",
            field->get_field_name());
        if (!field->check_type(krpc_field_type_string) &&
            !field->check_type(krpc_field_type_f32)    &&
            !field->check_type(krpc_field_type_f64)) {
            source.replace(
                "\t\tss << white << \"  {{@name}}=\" << (uint64_t){{@name}}[i] << std::endl;\n",
                field->get_field_name());
        } else {
            source.replace(
                "\t\tss << white << \"  {{@name}}=\" << {{@name}}[i] << std::endl;\n",
                field->get_field_name());
        }
        source << "\t}\n"
                << "\tss << white << \"]\" << std::endl;\n"; 
    }
}

void krpc_gen_cpp_t::gen_attribute_method_print_impl_table(
    krpc_field_t* field, krpc_ostream_t& source) {
        source.write(
            "\tss << white << \"{{@name}}<\" << std::endl;\n"
            "\tdo {\n"
            "\t\tstd::map<{{@key_type}}, {{@value_type}}>::iterator guard = {{@name}}.begin();\n"
            "\t\tfor (; guard != {{@name}}.end(); guard++) {\n",
            field->get_field_name().c_str(),
            field_find_type_name(field).c_str(),
            field->check_value_type(krpc_field_type_attribute) ?
                field->get_value_type_name().c_str() : field_find_value_type_name(field).c_str(),
            field->get_field_name().c_str(),
            field->get_field_name().c_str());
        if (!field->check_key_type(krpc_field_type_string) &&
            !field->check_key_type(krpc_field_type_f32)    &&
            !field->check_key_type(krpc_field_type_f64)) {
            source.write("\t\t\tss << white + \"  \" << \"key:\" << (uint64_t)guard->first << std::endl;\n");
        } else {
            source.write("\t\t\tss << white + \"  \" << \"key:\" << guard->first << std::endl;\n");
        }
        if (field->check_value_type(krpc_field_type_attribute)) {
            source.write(
                "\t\t\tss << white + \"  \" << \"value:\";\n"
                "\t\t\tss << std::endl;\n"
                "\t\t\tguard->second.print(ss, white + \"    \");\n");
        } else {
            source.write(
                "\t\t\tss << white + \"  \" << \"value:\" << guard->second << std::endl;\n");
        }
        source.write(
            "\t\t}\n"
            "\t} while(0);\n"
            "\tss << white << \">\" << std::endl;\n");
}

void krpc_gen_cpp_t::gen_attribute_method_print_impl_common(
    krpc_field_t* field, krpc_ostream_t& source) {
    if (field->check_type(krpc_field_type_attribute)) {
        source.write(
            "\t{{@name}}.print(ss, white);\n",
            field->get_field_name().c_str());
    } else {
        if (!field->check_type(krpc_field_type_string) &&
            !field->check_type(krpc_field_type_f32)    &&
            !field->check_type(krpc_field_type_f64)) {
                source.replace(
                    "\tss << white << \"{{@name}}=\" << (uint64_t){{@name}} << std::endl;\n",
                    field->get_field_name().c_str());
        } else {
            source.replace(
                "\tss << white << \"{{@name}}=\" << {{@name}} << std::endl;\n",
                field->get_field_name().c_str());
        }
    }
}

void krpc_gen_cpp_t::gen_attribute_method_print_impl(
    krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source.replace(
        "void {{@name}}::print(std::stringstream& ss, std::string white) {\n"
        "\tss << white << \"{{@name}}:\" << std::endl;\n"
        "\twhite += \"  \";\n",
        attribute->get_name());
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            gen_attribute_method_print_impl_array(*field, source);
        } else if ((*field)->check_table()) {
            gen_attribute_method_print_impl_table(*field, source);
        } else {
            gen_attribute_method_print_impl_common(*field, source);
        }
    }
    source << "}\n\n";
}

void krpc_gen_cpp_t::gen_struct_field_decl(krpc_ostream_t& header,
    krpc_field_t* field) {
    header << "\t";
    if (field->check_array()) {
        // 生成数组类型
        header << "std::vector<" << field_find_type_name(field) << ">";
    } else if (field->check_table()) {
        header << "std::map<" << field_find_type_name(field) << ","
               << field_find_value_type_name(field) << ">";
    } else {
        // 普通变量
        header << field_find_type_name(field);
    }
    header << " " << field->get_field_name() << ";";
    if (!field->get_comment().empty()) {
        header << " ///< " << field->get_comment();
    }
    header << "\n";
}

void krpc_gen_cpp_t::gen_rpc_call_param_decl(krpc_ostream_t& header,
    krpc_field_t* field) {
    if (field->check_array()) {
        // 生成数组类型引用
        header << "std::vector<" << field_find_type_name(field) << ">&";
    } else if (field->check_table()) {
        header << "std::map<" << field_find_type_name(field) << ","
               << field_find_value_type_name(field) << ">&";
    } else {
        // 普通变量
        header << param_find_type_name(field);
    }
    header << " " << field->get_field_name();
}

std::string krpc_gen_cpp_t::field_find_type_name(krpc_field_t* field) {
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
    return field->get_field_type_name().c_str();
}

std::string krpc_gen_cpp_t::field_find_value_type_name(krpc_field_t* field) {
    if (field->check_value_type(krpc_field_type_i8)) {
        return "int8_t";
    } else if (field->check_value_type(krpc_field_type_i16)) {
        return "int16_t";
    } else if (field->check_value_type(krpc_field_type_i32)) {
        return "int32_t";
    } else if (field->check_value_type(krpc_field_type_i64)) {
        return "int64_t";
    } else if (field->check_value_type(krpc_field_type_ui8)) {
        return "uint8_t";
    } else if (field->check_value_type(krpc_field_type_ui16)) {
        return "uint16_t";
    } else if (field->check_value_type(krpc_field_type_ui32)) {
        return "uint32_t";
    } else if (field->check_value_type(krpc_field_type_ui64)) {
        return "uint64_t";
    } else if (field->check_value_type(krpc_field_type_f32)) {
        return "float";
    } else if (field->check_value_type(krpc_field_type_f64)) {
        return "double";
    } else if (field->check_value_type(krpc_field_type_string)) {
        return "std::string";
    }
    return field->get_value_type_name().c_str();
}

std::string krpc_gen_cpp_t::param_find_table_key_set_func_name(krpc_field_t* field) {
    if (field->check_key_type(krpc_field_type_i8)) {
        return "krpc_number_set_i8";
    } else if (field->check_key_type(krpc_field_type_i16)) {
        return "krpc_number_set_i16";
    } else if (field->check_key_type(krpc_field_type_i32)) {
        return "krpc_number_set_i32";
    } else if (field->check_key_type(krpc_field_type_i64)) {
        return "krpc_number_set_i64";
    } else if (field->check_key_type(krpc_field_type_ui8)) {
        return "krpc_number_set_ui8";
    } else if (field->check_key_type(krpc_field_type_ui16)) {
        return "krpc_number_set_ui16";
    } else if (field->check_key_type(krpc_field_type_ui32)) {
        return "krpc_number_set_ui32";
    } else if (field->check_key_type(krpc_field_type_ui64)) {
        return "krpc_number_set_ui64";
    } else if (field->check_key_type(krpc_field_type_f32)) {
        return "krpc_number_set_f32";
    } else if (field->check_key_type(krpc_field_type_f64)) {
        return "krpc_number_set_f64";
    } else if (field->check_key_type(krpc_field_type_string)) {
        return "krpc_string_set";
    }
    return "";
}

std::string krpc_gen_cpp_t::param_find_table_key_get_func_name(krpc_field_t* field) {
    if (field->check_key_type(krpc_field_type_i8)) {
        return "krpc_number_get_i8";
    } else if (field->check_key_type(krpc_field_type_i16)) {
        return "krpc_number_get_i16";
    } else if (field->check_key_type(krpc_field_type_i32)) {
        return "krpc_number_get_i32";
    } else if (field->check_key_type(krpc_field_type_i64)) {
        return "krpc_number_get_i64";
    } else if (field->check_key_type(krpc_field_type_ui8)) {
        return "krpc_number_get_ui8";
    } else if (field->check_key_type(krpc_field_type_ui16)) {
        return "krpc_number_get_ui16";
    } else if (field->check_key_type(krpc_field_type_ui32)) {
        return "krpc_number_get_ui32";
    } else if (field->check_key_type(krpc_field_type_ui64)) {
        return "krpc_number_get_ui64";
    } else if (field->check_key_type(krpc_field_type_f32)) {
        return "krpc_number_get_f32";
    } else if (field->check_key_type(krpc_field_type_f64)) {
        return "krpc_number_get_f64";
    } else if (field->check_key_type(krpc_field_type_string)) {
        return "krpc_string_get";
    }
    return "";
}

std::string krpc_gen_cpp_t::param_find_table_value_get_func_name(krpc_field_t* field) {
    if (field->check_value_type(krpc_field_type_i8)) {
        return "krpc_number_get_i8";
    } else if (field->check_value_type(krpc_field_type_i16)) {
        return "krpc_number_get_i16";
    } else if (field->check_value_type(krpc_field_type_i32)) {
        return "krpc_number_get_i32";
    } else if (field->check_value_type(krpc_field_type_i64)) {
        return "krpc_number_get_i64";
    } else if (field->check_value_type(krpc_field_type_ui8)) {
        return "krpc_number_get_ui8";
    } else if (field->check_value_type(krpc_field_type_ui16)) {
        return "krpc_number_get_ui16";
    } else if (field->check_value_type(krpc_field_type_ui32)) {
        return "krpc_number_get_ui32";
    } else if (field->check_value_type(krpc_field_type_ui64)) {
        return "krpc_number_get_ui64";
    } else if (field->check_value_type(krpc_field_type_f32)) {
        return "krpc_number_get_f32";
    } else if (field->check_value_type(krpc_field_type_f64)) {
        return "krpc_number_get_f64";
    } else if (field->check_value_type(krpc_field_type_string)) {
        return "krpc_string_get";
    }
    return "";
}

std::string krpc_gen_cpp_t::param_find_table_value_set_func_name(krpc_field_t* field) {
    if (field->check_value_type(krpc_field_type_i8)) {
        return "krpc_number_set_i8";
    } else if (field->check_value_type(krpc_field_type_i16)) {
        return "krpc_number_set_i16";
    } else if (field->check_value_type(krpc_field_type_i32)) {
        return "krpc_number_set_i32";
    } else if (field->check_value_type(krpc_field_type_i64)) {
        return "krpc_number_set_i64";
    } else if (field->check_value_type(krpc_field_type_ui8)) {
        return "krpc_number_set_ui8";
    } else if (field->check_value_type(krpc_field_type_ui16)) {
        return "krpc_number_set_ui16";
    } else if (field->check_value_type(krpc_field_type_ui32)) {
        return "krpc_number_set_ui32";
    } else if (field->check_value_type(krpc_field_type_ui64)) {
        return "krpc_number_set_ui64";
    } else if (field->check_value_type(krpc_field_type_f32)) {
        return "krpc_number_set_f32";
    } else if (field->check_value_type(krpc_field_type_f64)) {
        return "krpc_number_set_f64";
    } else if (field->check_value_type(krpc_field_type_string)) {
        return "krpc_string_set";
    }
    return "";
}

std::string krpc_gen_cpp_t::param_find_type_name(krpc_field_t* field) {
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
    } else if (field->check_type(krpc_field_type_table)) {
        std::string type = "const std::map<";
        type += field_find_type_name(field);
        type += ",";
        type += field_find_value_type_name(field);
        type += ">&";
        return type;
    }
    return field->get_field_type_name() + "&";
}
