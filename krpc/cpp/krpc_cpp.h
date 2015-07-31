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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KRPC_CPP_H
#define KRPC_CPP_H

#include <cstdint>
#include "krpc_ostream.h"

class krpc_gen_t;
class krpc_field_t;
class krpc_attribute_t;
class krpc_rpc_call_t;
class krpc_parser_t;

/**
 * C++代码生成类
 */
class krpc_gen_cpp_t {
public:
    /**
     * 构造
     * @param rpc_gen krpc_gen_t实例
     */
    krpc_gen_cpp_t(krpc_gen_t* rpc_gen);

    /**
     * 析构
     */
    ~krpc_gen_cpp_t();

    /**
     * 生成代码
     */
    void gen_code();

private:
    //
    // 头文件生成
    //

    void gen_header_file();
    void gen_struct_pre_decls(krpc_ostream_t& header);
    void gen_struct_decls(krpc_ostream_t& header);
    void gen_struct_decl(krpc_ostream_t& header, krpc_attribute_t* object);
    void gen_struct_field_decl(krpc_ostream_t& header, krpc_field_t* field);
    void gen_struct_method_decl(krpc_ostream_t& header, krpc_attribute_t* object);
    void gen_entry_decl(krpc_ostream_t& header);
    void gen_entry_rpc_method_decls(krpc_ostream_t& header);
    void gen_entry_rpc_method_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call);
    void gen_entry_rpc_method_comment(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call);
    void gen_entry_rpc_method_param_comment(krpc_ostream_t& header, krpc_field_t* field);
    void gen_struct_marshal_unmarshal_method_decls(krpc_ostream_t& header);
    void gen_struct_marshal_unmarshal_method_decl(krpc_ostream_t& header, krpc_attribute_t* object);
    void gen_rpc_call_proxy_decls(krpc_ostream_t& header);
    void gen_rpc_call_proxy_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call);
    void gen_rpc_call_stub_decls(krpc_ostream_t& header);
    void gen_rpc_call_stub_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call);
    void gen_rpc_call_decls(krpc_ostream_t& header);
    void gen_rpc_call_decl(krpc_ostream_t& header, krpc_rpc_call_t* rpc_call);

    //
    // 实现文件生成
    //

    void gen_source_file();
    void gen_entry_impls(krpc_ostream_t& source);
    void gen_entry_ctor(krpc_ostream_t& source);
    void gen_entry_rpc_call_wrapper_impls(krpc_ostream_t& source);
    void gen_entry_rpc_call_wrapper_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call, int rpcid);
    void gen_entry_rpc_call_wrapper_method_prototype(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call);
    void gen_entry_rpc_call_wrapper_method_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call, int rpcid);
    void gen_struct_marshal_method_impls(krpc_ostream_t& source);
    void gen_struct_marshal_method_impl(krpc_ostream_t& source, krpc_attribute_t* object);
    void gen_struct_marshal_field_impl(krpc_ostream_t& source, krpc_field_t* field);
    void gen_struct_unmarshal_method_impls(krpc_ostream_t& source);
    void gen_struct_unmarshal_method_impl(krpc_ostream_t& source, krpc_attribute_t* object);
    void gen_struct_unmarshal_field_impl(krpc_ostream_t& source, krpc_field_t* field, const std::string& name, int index);
    void gen_struct_method_impls(krpc_ostream_t& source);
    void gen_struct_method_impl(krpc_ostream_t& source, krpc_attribute_t* object);
    void gen_rpc_call_proxy_impls(krpc_ostream_t& source);
    void gen_rpc_call_proxy_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call);
    void gen_rpc_call_stub_impls(krpc_ostream_t& source);
    void gen_rpc_call_stub_impl(krpc_ostream_t& source, krpc_rpc_call_t* rpc_call);

    /**
     * RPC参数 - 声明
     */
    void gen_rpc_call_param_decl(krpc_ostream_t& header,
        krpc_field_t* field);

    /**
     * 属性屏幕输出
     */
    void gen_attribute_method_print_impl(krpc_attribute_t* attribute,
        krpc_ostream_t& source);

    /**
     * 字段 - marshal定义
     */
    void gen_field_marshal_impl(krpc_field_t* field,
        krpc_ostream_t& source, bool param = false);

    /**
     * 字段 - 非数组 - marshal定义
     */
    void gen_field_marshal_impl_not_array(krpc_field_t* field,
        krpc_ostream_t& source, const std::string& holder_name = "",
            const std::string& vector_name = "",
                const std::string& whites = "");

    /**
     * 字段 - 设置 - marshal定义
     */
	void gen_field_marshal_impl(krpc_field_t* field,
		krpc_ostream_t& source, const std::string& holder, const std::string& v, 
		const std::string& whites);
    /**
     * 字段 - unmarshal定义
     */
    void gen_field_unmarshal_impl(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 字段 - 非数组 - marshal定义
     */
    void gen_field_unmarshal_impl_not_array(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 字段 - 数组 - unmarshal定义
     */
    void gen_field_unmarshal_impl_array(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 字段 - 表 - unmarshal定义
     */
    void gen_field_unmarshal_impl_table(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 打印数组
     */
    void gen_attribute_method_print_impl_array(
        krpc_field_t* field, krpc_ostream_t& source);

    /**
     * 打印表
     */
    void gen_attribute_method_print_impl_table(
        krpc_field_t* field, krpc_ostream_t& source);

    /**
     * 打印非数组
     */
    void gen_attribute_method_print_impl_common(
        krpc_field_t* field, krpc_ostream_t& source);

    /**
     * 取得属性字段类型名
     */
    std::string field_find_type_name(krpc_field_t* field);

    /**
     * 取得表值类型名
     */
    std::string field_find_value_type_name(krpc_field_t* field);

    /**
     * 取得参数字段类型名
     */
    std::string param_find_type_name(krpc_field_t* field);

private:
    krpc_gen_t*    _rpc_gen; // 代码生成入口类
    krpc_parser_t* _parser;  // 解析器
    uint16_t       _rpc_id;  // RPC函数ID
};

#endif // KRPC_CPP_H
