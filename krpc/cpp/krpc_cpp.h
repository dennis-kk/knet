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

#ifndef KRPC_CPP_H
#define KRPC_CPP_H

#include <cstdint>
#include "krpc_ostream.h"

class krpc_gen_t;
class krpc_field_t;
class krpc_attribute_t;

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
    void lang_gen_code();

private:
    /**
     * 生成RPC调用框架类
     */
    void lang_gen_framework(krpc_ostream_t& header, krpc_ostream_t& source);

    /**
     * 生成RPC调用框架类 - 声明
     */
    void lang_gen_framework_decls(krpc_ostream_t& header);

    /**
     * 生成RPC调用框架类 - 定义
     */
    void lang_gen_framework_impls(krpc_ostream_t& source);

    /**
     * 包含头文件
     */
    void lang_gen_includes(krpc_ostream_t& header, krpc_ostream_t& source);

    /**
     * 属性对象预先声明
     */
    void lang_gen_attributes_pre_decls(krpc_ostream_t& header);

    /**
     * 属性对象相关代码生成
     */
    void lang_gen_attributes(krpc_ostream_t& header, krpc_ostream_t& source);

    /**
     * 属性对象相关代码生成 - 声明
     */
    void lang_gen_attribute_method_decl(krpc_attribute_t* attribute,
        krpc_ostream_t& header);

    /**
     * 属性对象相关代码生成 - 定义
     */
    void lang_gen_attribute_method_impl(krpc_attribute_t* attribute,
        krpc_ostream_t& source);

    /**
     * 属性字段 - 声明
     */
    void lang_gen_attribute_field_decl(krpc_ostream_t& header,
        krpc_field_t* field);

    /**
     * RPC函数 - 声明
     */
    void lang_gen_rpc_call_decls(krpc_ostream_t& header);

    /**
     * RPC函数 - 定义
     */
    void lang_gen_rpc_call_impls(krpc_ostream_t& source);

    /**
     * RPC参数 - 声明
     */
    void lang_gen_rpc_call_param_decl(krpc_ostream_t& header,
        krpc_field_t* field);

    /**
     * RPC函数 - proxy定义
     */
    void lang_gen_rpc_call_impl_proxy(krpc_attribute_t* attribute,
        krpc_ostream_t& source);

    /**
     * RPC函数 - stub定义
     */
    void lang_gen_rpc_call_impl_stub(krpc_attribute_t* attribute,
        krpc_ostream_t& source, const std::string& rpc_name);

    /**
     * 所有属性对象 - marshal/unmarshal声明
     */
    void lang_gen_attribute_marshal_method_decls(krpc_ostream_t& header);

    /**
     * 属性对象 - marshal声明
     */
    void lang_gen_attribute_marshal_method_decl(krpc_attribute_t* attribute,
        krpc_ostream_t& header);

    /**
     * 所有属性对象 - marshal定义
     */
    void lang_gen_attribute_marshal_method_impls(krpc_ostream_t& source);

    /**
     * 属性屏幕输出
     */
    void lang_gen_attribute_method_print_impl(krpc_attribute_t* attribute,
        krpc_ostream_t& source);

    /**
     * 属性对象 - marshal定义
     */
    void lang_gen_attribute_marshal_method_impl(krpc_attribute_t* attribute,
        krpc_ostream_t& source);

    /**
     * 所有属性对象 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_method_impls(krpc_ostream_t& source);

    /**
     * 属性对象 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_method_impl(krpc_attribute_t* attribute,
        krpc_ostream_t& source);

    /**
     * 属性对象 - 非数组 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_field_not_array(krpc_field_t* field,
        krpc_ostream_t& source, const std::string& name, int index);

    /**
     * 属性对象 - 非数组获取 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_field_not_array_get(
        krpc_ostream_t& source, const std::string& name,
            const std::string& method_name, int index);

    /**
     * 属性对象 - 数组 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_field_array(krpc_field_t* field,
        krpc_ostream_t& source, const std::string& name, int index);

    /**
     * 属性对象 — 数组元素 - unmarshal
     */
    void lang_gen_attribute_unmarshal_field_array_element(
        krpc_ostream_t& source, const std::string& name,
            const std::string& type_name);

    /**
     * 字段 - marshal定义
     */
    void lang_gen_field_marshal_impl(krpc_field_t* field,
        krpc_ostream_t& source, bool param = false);

    /**
     * 字段 - 非数组 - marshal定义
     */
    void lang_gen_field_marshal_impl_not_array(krpc_field_t* field,
        krpc_ostream_t& source, const std::string& holder_name = "",
            const std::string& vector_name = "",
                const std::string& whites = "");

    /**
     * 字段 - 设置 - marshal定义
     */
    void lang_gen_field_marshal_impl_not_array_set(krpc_field_t* field,
        krpc_ostream_t& source, const std::string& holder,
            const std::string& v, const std::string& method_name,
                const std::string& suffix, const std::string& whites);

    /**
     * 字段 - 数组 - marshal定义
     */
    void lang_gen_field_marshal_impl_array(krpc_field_t* field,
        krpc_ostream_t& source, bool param, const std::string& whites = "");

    /**
     * 字段 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 字段 - 非数组 - marshal定义
     */
    void lang_gen_field_unmarshal_impl_not_array(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 字段 - 非数组, 作为函数参数内调用 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl_not_array_inline(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 字段 - 非数组获取, 作为函数参数内调用 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl_not_array_inline_get(
        krpc_ostream_t& source, int index,
            const std::string& method_name);

    /**
     * 字段 - 非数组获取 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl_not_array_get(krpc_ostream_t& source,
        int index, const std::string& method_name);

    /**
     * 字段 - 数组 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl_array(krpc_field_t* field,
        krpc_ostream_t& source, int index);

    /**
     * 取得属性字段类型名
     */
    std::string lang_field_find_type_name(krpc_field_t* field);

    /**
     * 取得参数字段类型名
     */
    std::string lang_param_find_type_name(krpc_field_t* field);

private:
    krpc_gen_t* _rpc_gen; // 代码生成入口类
    uint16_t    _rpc_id;  // RPC函数ID
};

#endif // KRPC_CPP_H
