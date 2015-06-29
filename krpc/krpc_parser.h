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

#ifndef KRPC_PARSER_H
#define KRPC_PARSER_H

#include <list>
#include <map>
#include <exception>
#include <string>
#include "krpc_lexer.h"

class krpc_field_t;

/*
 * 字段类型
 */
enum {
    krpc_field_type_i8        = 1,
    krpc_field_type_i16       = 2,
    krpc_field_type_i32       = 4,
    krpc_field_type_i64       = 8,
    krpc_field_type_ui8       = 16,
    krpc_field_type_ui16      = 32,
    krpc_field_type_ui32      = 64,
    krpc_field_type_ui64      = 128,
    krpc_field_type_f32       = 256,
    krpc_field_type_f64       = 512,
    krpc_field_type_string    = 1024,
    krpc_field_type_array     = 2048,
    krpc_field_type_attribute = 4096,
    krpc_field_type_rpc       = 8192,
};

/*
 * 属性
 */
class krpc_attribute_t {
public:
    typedef std::list<krpc_field_t*> field_list_t;
    typedef field_list_t::iterator   iterator;

public:
    /*
     * 构造
     * @param name 属性名
     */
    krpc_attribute_t(const std::string& name);

    /*
     * 析构
     */
    ~krpc_attribute_t();
    
    /*
     * 增加属性字段
     * @param field krpc_field_t实例
     */
    void push_field(krpc_field_t* field);

    /*
     * 获取属性名
     * @return 属性名
     */
    const std::string& get_name();

    /*
     * 获取字段列表
     * @return 字段列表
     */
    field_list_t& get_field_list();

private:
    field_list_t _fields; // 字段列表
    std::string  _name;   // 属性名
};

/*
 * 字段
 */
class krpc_field_t {
public:
    /*
     * 构造
     * @param type 字段类型
     */
    krpc_field_t(int type);

    /*
     * 析构
     */
    ~krpc_field_t();

    /*
     * 设置字段类型
     * @param type 字段类型
     */
    void set_type(int type);

    /*
     * 检查字段是否是数组
     * @retval true 是数组
     * @retval false 不是数组
     */
    bool check_array();

    /*
     * 检查字段类型
     * @param type 字段类型
     * @retval true 是
     * @retval false 不是
     */
    bool check_type(int type);

    /*
     * 设置字段名
     * @param field_name 字段名
     */
    void set_field_name(const std::string& field_name);

    /*
     * 设置字段类型名
     * @param field_type 字段类型名
     */
    void set_field_type(const std::string& field_type);

    /*
     * 取得字段名
     * @return 字段名
     */
    const std::string& get_field_name();

    /*
     * 取得字段类型名
     * @return 字段类型名
     */
    const std::string& get_field_type();

private:
    /*
     * 转换token类型到字段类型
     * @return 字段类型
     */
    int convert(int type);

private:
    std::string _field_name; // 字段名
    std::string _field_type; // 字段类型名
    int         _type;       // 字段类型
};

/*
 * RPC调用
 */
class krpc_rpc_call_t {
public:
    /*
     * 构造
     * @param rpc_name 函数名称
     */
    krpc_rpc_call_t(const std::string& rpc_name);

    /*
     * 析构
     */
    ~krpc_rpc_call_t();

    /*
     * 取得函数名
     * @return 函数名
     */
    const std::string& get_name();

    /*
     * 取得参数表
     * @return krpc_attribute_t实例
     */
    krpc_attribute_t* get_attribute();

private:
    std::string       _name;      // 函数名
    krpc_attribute_t* _attribute; // 参数表
};

/*
 * 语法解析器
 */
class krpc_parser_t {
public:
    typedef std::map<std::string, krpc_attribute_t*> object_map_t;
    typedef std::map<std::string, krpc_rpc_call_t*>  rpc_call_map_t;

public:
    /*
     * 构造
     * @param parent 父解析器
     * @param dir 文件所在目录
     * @param file_name 文件名
     */
    krpc_parser_t(krpc_parser_t* parent, const char* dir,
        const char* file_name);

    /*
     * 析构
     */
    ~krpc_parser_t();

    /*
     * 解析
     */
    void parse() throw(std::exception);

    /*
     * 获取属性列表
     * @return 属性列表
     */
    object_map_t& get_attributes();

    /*
     * 获取RPC函数列表
     * @return RPC函数列表
     */
    rpc_call_map_t& get_rpc_calls();

private:
    /*
     * 取得下一个token
     * @return krpc_token_t实例
     */
    krpc_token_t* next_token();

    /*
     * 解析chunk
     * @param token 当前token
     */
    void parse_trunk(krpc_token_t* token);

    /*
     * 解析属性
     * @param token 当前token
     * @return krpc_attribute_t实例
     */
    krpc_attribute_t* parse_attribute(krpc_token_t* token);

    /*
     * 解析RPC调用
     * @param token 当前token
     * @return krpc_rpc_call_t实例
     */
    krpc_rpc_call_t* parse_rpc_call(krpc_token_t* token);

    /*
     * 解析导入文件
     * @param token 当前token
     */
    void parse_import(krpc_token_t* token);

    /*
     * 解析字段
     * @param token 当前token
     * @return krpc_field_t实例
     */
    krpc_field_t* parse_field(krpc_token_t* token);

private:
    object_map_t   _objects;        // 属性表
    rpc_call_map_t _rpc_calls;      // RPC函数表
    std::string    _file_name;      // 文件名
    std::string    _dir;            // 当前目录
    krpc_lexer_t*  _lexer;          // token解析器
    krpc_parser_t* _parent;         // 父解析器
    krpc_parser_t* _current_parser; // 当前解析器
};

#endif // KRPC_PARSER_H
