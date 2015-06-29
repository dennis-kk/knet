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
  _rpc_id(1) {
}

krpc_gen_cpp_t::~krpc_gen_cpp_t() {
}

void krpc_gen_cpp_t::lang_gen_code() {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    // 头文件，实现文件路径
    krpc_ostream_t header(options["dir"] + options["name"] + ".h");
    krpc_ostream_t source(options["dir"] + options["name"] + ".cpp");
    header.write(
        "//\n"
        "// KRPC - Generated code, *DO NOT CHANGE*\n"
        "//\n\n");
    source.write(
        "//\n"
        "// KRPC - Generated code, *DO NOT CHANGE*\n"
        "//\n\n");
    header.replace(
        "#ifndef _krpc_@file_name@_h_\n"
        "#define _krpc_@file_name@_h_\n\n",
        options["name"]);
    lang_gen_includes(header, source);                 // 包含文件
    header.write("namespace @name@ {\n\n", options["name"].c_str());
    source.write("namespace @name@ {\n\n", options["name"].c_str());
    lang_gen_attributes_pre_decls(header);             // 预先声明
    lang_gen_attributes(header, source);               // 属性对象
    lang_gen_rpc_call_decls(header);                   // RPC声明
    lang_gen_rpc_call_impls(source);                   // RPC定义
    lang_gen_attribute_marshal_method_impls(source);   // marshal方法
    lang_gen_attribute_unmarshal_method_impls(source); // unmarshal方法
    lang_gen_framework(header, source);                // 入口框架
    header.replace(
        "inline static @file_name@_t* @file_name@_ptr() {\n"
        "\treturn @file_name@_t::instance();\n"
        "}\n\n",
        options["name"]);
    header << "}\n\n";
    source << "}\n\n";
    header.replace("#endif // _krpc_@file_name@_h_\n", options["name"]);
}

void krpc_gen_cpp_t::lang_gen_includes(krpc_ostream_t& header, krpc_ostream_t& source) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    header.write(
        "#include <cstdint>\n"
        "#include <string>\n" 
        "#include <vector>\n"
        "#include \"knet.h\"\n\n");
    source.write(
        "#include <sstream>\n"
        "#include \"@name@.h\"\n\n",
        options["name"].c_str());
}

void krpc_gen_cpp_t::lang_gen_framework(krpc_ostream_t& header,
    krpc_ostream_t& source) {
    lang_gen_framework_decls(header);
    lang_gen_framework_impls(source);
}

void krpc_gen_cpp_t::lang_gen_framework_decls(krpc_ostream_t& header) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    header.replace(
        "/*\n"
        " * RPC单件类\n"
        " */\n"
        "class @name@_t {\n"
        "public:\n"
        "\t/*\n"
        "\t * 析构\n"
        "\t */\n"
        "\t~@name@_t();\n\n"
        "\t/*\n"
        "\t * 取得单件指针\n"
        "\t * \\return @name@_t指针\n"
        "\t */\n"
        "\tstatic @name@_t* instance();\n\n"
        "\t/*\n"
        "\t * 销毁单件\n"
        "\t */\n"
        "\tstatic void finalize();\n\n"
        "\t/*\n"
        "\t * 从stream_t读取RPC调用请求\n"
        "\t * \\param stream stream_t实例\n"
        "\t * \\retval error_ok 成功处理一次RPC调用\n"
        "\t * \\retval error_rpc_not_enough_bytes 没有完整的RPC可以处理\n"
        "\t * \\retval error_rpc_unmarshal_fail 处理RPC包字节流时读取失败\n"
        "\t * \\retval error_rpc_unknown_id 读取到RPC调用，但RPC ID未注册\n"
        "\t * \\retval error_rpc_cb_fail 调用RPC处理函数时，处理函数内部发生错误\n"
        "\t * \\retval error_rpc_cb_fail_close 调用RPC处理函数时，处理函数内部发生错误，处理函数要求关闭stream_t相关联的管道\n"
        "\t * \\retval error_rpc_cb_close 调用RPC处理函数后，处理函数要求关闭stream_t相关联的管道\n"
        "\t * \\retval error_rpc_unknown_type RPC类型错误\n"
        "\t */\n"
        "\tint rpc_proc(stream_t* stream); \n\n",
        options["name"]);
    // RPC包装方法
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        header.write(
            "\t/*\n"
            "\t * @rpc@ @method_comment@\n",
            rpc_call->second->get_name().c_str(),
            rpc_call->second->get_comment().empty() ?
                "代理调用包装方法" : rpc_call->second->get_comment().c_str());
        krpc_attribute_t* attribute = rpc_call->second->get_attribute();
        krpc_attribute_t::field_list_t::iterator field =
            attribute->get_field_list().begin();
        for (; field != attribute->get_field_list().end(); field++) {
            if ((*field)->get_comment().empty()) {
                header.write("\t * \\param @field@ @comment@\n",
                    (*field)->get_field_name().c_str(),
                    "N/A.");
            } else {
                header.write("\t * \\param @field@ @comment@\n",
                    (*field)->get_field_name().c_str(),
                    (*field)->get_comment().c_str());
            }
        }
        header.write(
            "\t * \\retval error_ok 成功\n"
            "\t * \\retval error_rpc_marshal_fail 序列化RPC调用时失败\n"
            "\t */\n");
        header << "\tint " << rpc_call->first << "(stream_t* stream, ";
        attribute = rpc_call->second->get_attribute();
        field = attribute->get_field_list().begin();
        size_t size = attribute->get_field_list().size();
        for (size_t pos = 0; field != attribute->get_field_list().end();
            field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header.write(");\n\n");
    }
    // 私有成员
    header.replace(
        "private:\n"
        "\t/*\n"
        "\t * 构造函数\n"
        "\t */\n"
        "\t@name@_t();\n\n"
        "\t/*\n"
        "\t * 拷贝构造\n"
        "\t */\n"
        "\t@name@_t(const @name@_t&);\n\n"
        "private:\n"
        "\tstatic @name@_t* _instance; // 单件指针\n"
        "\tkrpc_t* _rpc; // RPC实现类\n"
        "};\n\n",
        options["name"]);
}

void krpc_gen_cpp_t::lang_gen_framework_impls(krpc_ostream_t& source) {
    krpc_gen_t::option_map_t& options = _rpc_gen->get_options();
    source.replace(
        "@name@_t* @name@_t::_instance = 0;\n\n"
        "@name@_t::@name@_t() {\n"
        "\t_rpc = krpc_create();\n",
        options["name"]);
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        parser->get_rpc_calls().begin();
    // 注册stub
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++, _rpc_id++) {
        source << "\t" << "krpc_add_cb(_rpc, " << _rpc_id << ", "
               << rpc_call->first << "_stub);\n";
    }
    source.replace(
        "}\n\n"
        "@name@_t::~@name@_t() {\n"
        "\tkrpc_destroy(_rpc);\n"
        "}\n\n"
        "@name@_t* @name@_t::instance() {\n"
        "\tif (!_instance) {\n"
        "\t\t_instance = new @name@_t();\n"
        "\t}\n"
        "\treturn _instance;\n"
        "}\n\n"
        "void @name@_t::finalize() {\n"
        "\tif (_instance) {\n"
        "\t\tdelete _instance;\n"
        "\t}\n"
        "}\n\n"
        "int @name@_t::rpc_proc(stream_t* stream) {\n"
        "\treturn krpc_proc(_rpc, stream);\n"
        "}\n\n",
        options["name"]);
    // RPC包装方法
    rpc_call = parser->get_rpc_calls().begin();
    for (int rpcid = 1; rpc_call != parser->get_rpc_calls().end();
        rpc_call++, rpcid++) {
        source << "int " << options["name"] << "_t::"
               << rpc_call->first << "(stream_t* stream, ";
        krpc_attribute_t* attribute = rpc_call->second->get_attribute();
        krpc_attribute_t::field_list_t::iterator field =
            attribute->get_field_list().begin();
        size_t size = attribute->get_field_list().size();
        for (size_t pos = 0; field != attribute->get_field_list().end();
            field++, pos++) {
            lang_gen_rpc_call_param_decl(source, *field);
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << ") {\n"
               << "\tkrpc_object_t* o = "
               << rpc_call->first << "_proxy(";
        field = attribute->get_field_list().begin();
        size = attribute->get_field_list().size();
        for (size_t pos = 0; field != attribute->get_field_list().end();
            field++, pos++) {
            source << (*field)->get_field_name();
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << ");\n"
               << "\tint error = krpc_call(_rpc, stream, " << rpcid << ", o);\n"
               << "\tkrpc_object_destroy(o);\n"
               << "\treturn error;\n"
               << "}\n\n";
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_decls(krpc_ostream_t& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::rpc_call_map_t::iterator rpc_call =
        parser->get_rpc_calls().begin();
    for (; rpc_call != parser->get_rpc_calls().end(); rpc_call++) {
        header.write(
            "/*\n"
            " * @method_name@代理\n"
            " */\n",
            rpc_call->first.c_str());
        header << "krpc_object_t* " << rpc_call->first << "_proxy(";
        krpc_attribute_t* attribute = rpc_call->second->get_attribute();
        krpc_attribute_t::field_list_t::iterator field =
            attribute->get_field_list().begin();
        size_t size = attribute->get_field_list().size();
        for (size_t pos = 0; field != attribute->get_field_list().end();
            field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header << ");\n\n";
        header.write(
            "/*\n"
            " * @method_name@桩\n"
            " */\n",
            rpc_call->first.c_str());
        header << "int " << rpc_call->first << "_stub(krpc_object_t* o);\n\n";
        header.write(
            "/*\n"
            " * @method_name@声明，需实现此方法\n"
            " */\n",
            rpc_call->first.c_str());
        header << "int " << rpc_call->first << "(";
        field = attribute->get_field_list().begin();
        size = attribute->get_field_list().size();
        for (size_t pos = 0; field != attribute->get_field_list().end();
            field++, pos++) {
            lang_gen_rpc_call_param_decl(header, *field);
            if (pos + 1 < size) {
                header << ", ";
            }
        }
        header << ");\n\n";
    }
    header << "\n";
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impls(krpc_ostream_t& source) {
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
            lang_gen_rpc_call_param_decl(source, *field);
            if (pos + 1 < size) {
                source << ", ";
            }
        }
        source << ") {\n";
        lang_gen_rpc_call_impl_proxy(attribute, source);
        source << "\treturn v;\n"
               << "}\n\n";
        source << "int " << rpc_call->first << "_stub(krpc_object_t* o) {\n";
        lang_gen_rpc_call_impl_stub(attribute, source, rpc_call->first);
        source << "}\n\n";
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impl_proxy(krpc_attribute_t* attribute,
    krpc_ostream_t& source) {
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    size_t size = attribute->get_field_list().size();
    if (size > 0) {
        source << "\tkrpc_object_t* v = krpc_object_create();\n";
    }
    for (; field != attribute->get_field_list().end(); field++) {
        lang_gen_field_marshal_impl(*field, source, true);
    }
}

void krpc_gen_cpp_t::lang_gen_rpc_call_impl_stub(krpc_attribute_t* attribute,
    krpc_ostream_t& source, const std::string& rpc_name) {
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    int param = 0;
    for (; field != attribute->get_field_list().end(); field++, param++) {
        lang_gen_field_unmarshal_impl(*field, source, param);
    }
    source << "\treturn " << rpc_name << "(";
    for (int i = 0; i < param; i++) {
        if (i + 1 < param) {
            source << "p" << i << ", ";
        } else {
            source << "p" << i;
        }
    }
    source << ");\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl(krpc_field_t* field,
    krpc_ostream_t& source, int index) {
    if (field->check_array()) {
        lang_gen_field_unmarshal_impl_array(field, source, index);
    } else {
        lang_gen_field_unmarshal_impl_not_array(field, source, index);
    }
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_array(krpc_field_t* field,
    krpc_ostream_t& source, int index) {
    source << "\t" << "std::vector<" << lang_field_find_type_name(field)
           << "> p" << index  << ";\n"
           << "\t" << "do {\n"
           << "\t\t" << "krpc_object_t* v = krpc_vector_get(o, "
           << index << ");\n"
           << "\t\t" << "for (uint32_t i = 0; i < krpc_vector_get_size(v); i++) {\n";
    lang_gen_field_unmarshal_impl_not_array_inline(field, source, index);
    source << "\t\t" << "}\n"
           << "\t" << "} while(0);\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_inline_get(
    krpc_ostream_t& source, int index, const std::string& method_name) {
    source << "p" << index << ".push_back(" << method_name
           << "(krpc_vector_get(v, i)));\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_inline(
    krpc_field_t* field, krpc_ostream_t& source, int index) {
    source << "\t\t    ";
    if (field->check_type(krpc_field_type_attribute)) {
        source << lang_field_find_type_name(field) << " o_;\n";
        source << "\t\t    " << "unmarshal(krpc_vector_get(v, i), o_);\n";
        source << "\t\t    " << "p" << index << ".push_back(o_);\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_field_unmarshal_impl_not_array_inline_get(source, index,
            "krpc_string_get");
    }
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array_get(
    krpc_ostream_t& source, int index,
        const std::string& method_name) {
    source << "\tp" << index << " = " << method_name
           << "(krpc_vector_get(o, " << index << "));\n";
}

void krpc_gen_cpp_t::lang_gen_field_unmarshal_impl_not_array(
    krpc_field_t* field, krpc_ostream_t& source, int index) {
    source << "\t" << lang_field_find_type_name(field) << " p"
           << index  << ";\n";
    if (field->check_type(krpc_field_type_attribute)) {
        source << "\t" << "unmarshal(krpc_vector_get(o, "
               << index << "), p" << index << ");\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_field_unmarshal_impl_not_array_get(source, index,
            "krpc_string_get");
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_decls(
    krpc_ostream_t& header) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {        
        lang_gen_attribute_marshal_method_decl(object->second, header);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_impls(
    krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        lang_gen_attribute_marshal_method_impl(object->second, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_method_impls(
    krpc_ostream_t& source) {
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {        
        lang_gen_attribute_unmarshal_method_impl(object->second, source);
    }
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_decl(
    krpc_attribute_t* attribute,
    krpc_ostream_t& header) {
    header.write(
        "/*\n"
        " * @attribute_name@序列化\n"
        " */\n",
        attribute->get_name().c_str());
    header << "krpc_object_t* marshal("
           << attribute->get_name()
           << "& o);\n\n";
     header.write(
        "/*\n"
        " * @attribute_name@反序列化\n"
        " */\n",
        attribute->get_name().c_str());
    header << "bool unmarshal(krpc_object_t* v, "
           << attribute->get_name()
           << "& o);\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_marshal_method_impl(
    krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source << "krpc_object_t* marshal(" << attribute->get_name() << "& o) {\n";
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    source << "\t" << "krpc_object_t* v = krpc_object_create();\n";
    for (int i = 0; field != attribute->get_field_list().end(); field++, i++) {
        if ((*field)->check_array()) {
            lang_gen_field_marshal_impl_array(*field, source, false);
        } else {
            std::string holder = "o.";
            holder += (*field)->get_field_name();
            lang_gen_field_marshal_impl_not_array(*field, source, holder, "v");
        }
    }
    source << "\treturn v;\n"
           << "}\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_method_impl(
    krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source << "bool unmarshal(krpc_object_t* v, " << attribute->get_name()
           << "& o) {\n";
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    for (int i = 0; field != attribute->get_field_list().end(); field++, i++) {
        std::string name = "o.";
        name += (*field)->get_field_name();
        if ((*field)->check_array()) {
            lang_gen_attribute_unmarshal_field_array(*field, source, name, i);
        } else {            
            lang_gen_attribute_unmarshal_field_not_array(*field, source, name, i);
        }
    }
    source << "\treturn true;\n"
           << "}\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_array_element(
    krpc_ostream_t& source, const std::string& name,
        const std::string& method_name) {
    source << "\t\t    " << name << ".push_back(" << method_name
           << "(krpc_vector_get(v_, i)));\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_array(
    krpc_field_t* field, krpc_ostream_t& source, const std::string& name,
        int index) {
    source << "\tdo {\n"
           << "\t\tkrpc_object_t* v_ = 0;\n"
           << "\t\tv_ = krpc_vector_get(v, " << index << ");\n"
           << "\t\tfor (uint32_t i = 0; i < krpc_vector_get_size(v_); i++) {\n";
    if (field->check_type(krpc_field_type_attribute)) {
        source.write(
            "\t\t    @file_type@ o_;\n"
            "\t\t    unmarshal(krpc_vector_get(v_, i), o_);\n"
            "\t\t    @name@.push_back(o_);\n",
            field->get_field_type().c_str(), name.c_str());
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i8");
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i16");
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i32");
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_i64");
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui8");
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui16");
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui32");
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_ui64");
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_f32");
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_number_get_f64");
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_attribute_unmarshal_field_array_element(source, name,
            "krpc_string_get");
    }
    source << "\t\t}\n";
    source << "\t} while(0);\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_not_array_get(
    krpc_ostream_t& source, const std::string& name,
        const std::string& method_name, int index) {
    source << "\t" << name << " = " << method_name << "(krpc_vector_get(v, "
           << index << "));\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_unmarshal_field_not_array(
    krpc_field_t* field, krpc_ostream_t& source, const std::string& name,
        int index) {
    if (field->check_type(krpc_field_type_attribute)) {
        source << "\t" << "unmarshal(krpv_vector_get(v, " << index << "), "
               << name << ");\n";
    } else if (field->check_type(krpc_field_type_i8)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i8", index);
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i16", index);
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i32", index);
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_i64", index);
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui8", index);
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui16", index);
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui32", index);
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_ui64", index);
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_f32", index);
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_number_get_f64", index);
    } else if (field->check_type(krpc_field_type_string)) {
        lang_gen_attribute_unmarshal_field_not_array_get(source, name,
            "krpc_string_get", index);
    }
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_array(krpc_field_t* field,
    krpc_ostream_t& source, bool param, const std::string& whites) {
    source << whites << "\tdo {\n";
    if (!param) {
        source << whites << "\t\tstd::vector<"
               << lang_field_find_type_name(field) << ">::iterator guard = o."
               << field->get_field_name() << ".begin();\n";
    } else {
        source << whites << "\t\tstd::vector<"
               << lang_field_find_type_name(field)
               << ">::iterator guard = " << field->get_field_name()
               << ".begin();\n";
    }
    source << whites << "\t\tkrpc_object_t* v_ = krpc_object_create();\n"
           << whites << "\t\tkrpc_vector_clear(v_);\n";
    if (!param) {
        source << whites << "\t\t" << "for(; guard != o."
               << field->get_field_name() << ".end(); guard++) {\n";
    } else {
        source << whites << "\t\t" << "for(; guard != "
               << field->get_field_name() << ".end(); guard++) {\n";
    }
    lang_gen_field_marshal_impl_not_array(field, source, "(*guard)", "v_",
        whites + "\t\t");
    source << whites << "\t\t}\n"
           << whites << "\t\tkrpc_vector_push_back(v, v_);\n"
           << whites << "\t} while(0);\n";
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_not_array_set(
    krpc_field_t* field, krpc_ostream_t& source, const std::string& holder,
        const std::string& v, const std::string& method_name,
            const std::string& suffix, const std::string& whites) {
    source << whites << "\t" << "krpc_object_t* " << field->get_field_name()
           << "_" << suffix << " = krpc_object_create();\n"
           << whites << "\t" << method_name << "(" << field->get_field_name()
           << "_" << suffix << ", " << holder << ");\n"
           << whites << "\t" << "krpc_vector_push_back(" << v << ", "
           << field->get_field_name() << "_" << suffix << ");\n";
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl_not_array(krpc_field_t* field,
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
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i8", "i8", whites);
    } else if (field->check_type(krpc_field_type_i16)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i16", "i16", whites);
    } else if (field->check_type(krpc_field_type_i32)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i32", "i32", whites);
    } else if (field->check_type(krpc_field_type_i64)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_i64", "i64", whites);
    } else if (field->check_type(krpc_field_type_ui8)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui8", "ui8", whites);
    } else if (field->check_type(krpc_field_type_ui16)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui16", "ui16", whites);
    } else if (field->check_type(krpc_field_type_ui32)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui32", "ui32", whites);
    } else if (field->check_type(krpc_field_type_ui64)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_ui64", "ui64", whites);
    } else if (field->check_type(krpc_field_type_f32)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_f32", "f32", whites);
    } else if (field->check_type(krpc_field_type_f64)) {
        lang_gen_field_marshal_impl_not_array_set(field, source, holder, v,
            "krpc_number_set_f64", "f64", whites);
    } else if (field->check_type(krpc_field_type_string)) {
        source << whites << "\tkrpc_object_t* " << field->get_field_name()
               << "_string = krpc_object_create();\n"
               << whites << "\tkrpc_string_set(" << field->get_field_name()
               << "_string, " << holder << ".c_str());\n"
               << whites << "\tkrpc_vector_push_back(" << v << ", "
               << field->get_field_name() << "_string);\n";
    }
}

void krpc_gen_cpp_t::lang_gen_field_marshal_impl(krpc_field_t* field,
    krpc_ostream_t& source, bool param) {
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

void krpc_gen_cpp_t::lang_gen_attribute_method_print_impl(
    krpc_attribute_t* attribute, krpc_ostream_t& source) {
    source.replace(
        "void @name@::print(std::stringstream& ss, std::string white) {\n"
        "\tss << white << \"@name@:\" << std::endl;\n"
        "\twhite += \"  \";\n", attribute->get_name());
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            if ((*field)->check_type(krpc_field_type_attribute)) {
                source.replace(
                    "\tss << white << \"@name@[\" << std::endl;\n"
                    "\tfor (size_t i = 0; i < @name@.size(); i++) {\n"
                    "\t\t@name@[i].print(ss, white + \"  \");\n"
                    "\t}\n"
                    "\tss << white << \"]\" << std::endl;\n",
                    (*field)->get_field_name());
            } else {
                source.replace(
                    "\tss << white << \"@name@[\" << std::endl;\n"
                    "\tfor (size_t i = 0; i < @name@.size(); i++) {\n",
                    (*field)->get_field_name());
                if (!(*field)->check_type(krpc_field_type_string) &&
                    !(*field)->check_type(krpc_field_type_f32)    &&
                    !(*field)->check_type(krpc_field_type_f64)) {
                    source.replace(
                        "\t\tss << white << \"  @name@=\" << (uint64_t)@name@[i] << std::endl;\n",
                        (*field)->get_field_name());
                } else {
                    source.replace(
                        "\t\tss << white << \"  @name@=\" << @name@[i] << std::endl;\n",
                        (*field)->get_field_name());
                }
                source << "\t}\n"
                       << "\tss << white << \"]\" << std::endl;\n"; 
            }
        } else {
            if ((*field)->check_type(krpc_field_type_attribute)) {
               source << "\t" << (*field)->get_field_name()
                      << ".print(ss, white);\n";
            } else {
                if (!(*field)->check_type(krpc_field_type_string) &&
                    !(*field)->check_type(krpc_field_type_f32)    &&
                    !(*field)->check_type(krpc_field_type_f64)) {
                    source << "\tss << white << \""
                           << (*field)->get_field_name() << "=\" << (uint64_t)"
                           << (*field)->get_field_name()
                           << " << std::endl;\n";
                } else {
                    source << "\tss << white << \"" << (*field)->get_field_name()
                           << "=\" << " << (*field)->get_field_name()
                           << " << std::endl;\n";
                }
            }
        }
    }
    source << "}\n\n";
}

void krpc_gen_cpp_t::lang_gen_attribute_method_impl(krpc_attribute_t* attribute,
    krpc_ostream_t& source) {
    source.replace(
        "@name@::@name@() {}\n\n"
        "@name@::@name@(const @name@& rht) {\n",
        attribute->get_name());
    krpc_attribute_t::field_list_t::iterator field =
        attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source.replace(
                "\tpush_back_all(@name@, rht.@name@.begin(), rht.@name@.end());\n",
                (*field)->get_field_name());
        } else {
            source.replace(
                "\t@name@ = rht.@name@;\n", (*field)->get_field_name());
        }
    }
    source << "}\n\n";
    // operator=()
    source.replace(
        "const @name@& @name@::operator=(const @name@& rht) {\n",
        attribute->get_name());
    field = attribute->get_field_list().begin();
    for (; field != attribute->get_field_list().end(); field++) {
        if ((*field)->check_array()) {
            source.replace(
                "\tpush_back_all(@name@, rht.@name@.begin(), rht.@name@.end());\n",
                (*field)->get_field_name());
        } else {
            source.replace(
                "\t@name@ = rht.@name@;\n",
                (*field)->get_field_name());
        }
    }
    source << "\treturn *this;\n"
           << "}\n\n";
    // print()
    lang_gen_attribute_method_print_impl(attribute, source);
}

void krpc_gen_cpp_t::lang_gen_attribute_method_decl(krpc_attribute_t* attribute,
    krpc_ostream_t& header) {
    header.replace(
        "\t/*\n"
        "\t * 构造函数\n"
        "\t */\n"
        "\t@name@();\n\n"
        "\t/*\n"
        "\t * 拷贝构造\n"
        "\t * \\param rht @name@引用\n"
        "\t */\n"
        "\t@name@(const @name@& rht);\n\n"
        "\t/*\n"
        "\t * 赋值\n"
        "\t * \\param rht @name@引用\n"
        "\t */\n"
        "\tconst @name@& operator=(const @name@& rht);\n\n"
        "\t/*\n"
        "\t * 打印对象\n"
        "\t * \\param ss std::stringstream引用， 对象信息将输出到ss\n"
        "\t * \\param white 缩进空格\n"
        "\t */\n"
        "\tvoid print(std::stringstream& ss, std::string white = \"\");\n",
        attribute->get_name());
}

void krpc_gen_cpp_t::lang_gen_attributes(krpc_ostream_t& header,
    krpc_ostream_t& source) {
    source.replace(
        "template<typename T>\n"
        "void push_back_all(std::vector<T>& v, typename std::vector<T>::const_iterator begin,\n"
        "\ttypename std::vector<T>::const_iterator end) {\n"
        "\tfor (; begin != end; begin++) {\n"
        "\t\tv.push_back(*begin);\n"
        "\t}\n"
        "}\n\n",
        _rpc_gen->get_options()["name"]);
    krpc_parser_t* parser = _rpc_gen->get_parser();
    krpc_parser_t::object_map_t::iterator object =
        parser->get_attributes().begin();
    for (; object != parser->get_attributes().end(); object++) {
        if (!object->second->get_comment().empty()) {
            header.write(
                "/*\n"
                " * @comment@\n"
                " */\n", object->second->get_comment().c_str());
        }
        header << "struct " << object->first << " {\n";
        krpc_attribute_t::field_list_t::iterator field =
            object->second->get_field_list().begin();
        for (; field != object->second->get_field_list().end(); field++) {
            lang_gen_attribute_field_decl(header, *field);
        }
        lang_gen_attribute_method_decl(object->second, header);
        lang_gen_attribute_method_impl(object->second, source);
        header << "};\n\n";
    }
    lang_gen_attribute_marshal_method_decls(header);
}

void krpc_gen_cpp_t::lang_gen_attribute_field_decl(krpc_ostream_t& header,
    krpc_field_t* field) {
    header << "\t";
    if (field->check_array()) {
        // 生成数组类型
        header << "std::vector<" << lang_field_find_type_name(field) << ">";
    } else {
        // 普通变量
        header << lang_field_find_type_name(field);
    }
    header << " " << field->get_field_name() << ";";
    if (!field->get_comment().empty()) {
        header << " ///< " << field->get_comment();
    }
    header << "\n";
}

void krpc_gen_cpp_t::lang_gen_rpc_call_param_decl(krpc_ostream_t& header,
    krpc_field_t* field) {
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
