#ifndef KRPC_CPP_H
#define KRPC_CPP_H

#include <sstream>
#include <cstdint>

class krpc_gen_t;
class krpc_field_t;
class krpc_attribute_t;

/*
 * C++代码生成类
 */
class krpc_gen_cpp_t {
public:
    /*
     * 构造
     * @param rpc_gen krpc_gen_t实例
     */
    krpc_gen_cpp_t(krpc_gen_t* rpc_gen);

    /*
     * 析构
     */
    ~krpc_gen_cpp_t();

    /*
     * 生成代码
     */
    void lang_gen_code();

private:
    /*
     * 生成RPC调用框架类
     */
    void lang_gen_framework(std::stringstream& header, std::stringstream& source);

    /*
     * 生成RPC调用框架类 - 声明
     */
    void lang_gen_framework_decls(std::stringstream& header);

    /*
     * 生成RPC调用框架类 - 定义
     */
    void lang_gen_framework_impls(std::stringstream& source);

    /*
     * 包含头文件
     */
    void lang_gen_includes(std::stringstream& header);

    /*
     * 属性对象预先声明
     */
    void lang_gen_attributes_pre_decls(std::stringstream& header);

    /*
     * 属性对象相关代码生成
     */
    void lang_gen_attributes(std::stringstream& header, std::stringstream& source);

    /*
     * 属性对象相关代码生成 - 声明
     */
    void lang_gen_attribute_method_decl(krpc_attribute_t* attribute, std::stringstream& header);

    /*
     * 属性对象相关代码生成 - 定义
     */
    void lang_gen_attribute_method_impl(krpc_attribute_t* attribute, std::stringstream& source);

    /*
     * 属性字段 - 声明
     */
    void lang_gen_attribute_field_decl(std::stringstream& header, krpc_field_t* field);

    /*
     * RPC函数 - 声明
     */
    void lang_gen_rpc_call_decls(std::stringstream& header);

    /*
     * RPC函数 - 定义
     */
    void lang_gen_rpc_call_impls(std::stringstream& source);

    /*
     * RPC参数 - 声明
     */
    void lang_gen_rpc_call_param_decl(std::stringstream& header, krpc_field_t* field);

    /*
     * RPC函数 - proxy定义
     */
    void lang_gen_rpc_call_impl_proxy(krpc_attribute_t* attribute, std::stringstream& source);

    /*
     * RPC函数 - stub定义
     */
    void lang_gen_rpc_call_impl_stub(krpc_attribute_t* attribute, std::stringstream& source, const std::string& rpc_name);

    /*
     * 所有属性对象 - marshal/unmarshal声明
     */
    void lang_gen_attribute_marshal_method_decls(std::stringstream& header);

    /*
     * 属性对象 - marshal声明
     */
    void lang_gen_attribute_marshal_method_decl(krpc_attribute_t* attribute, std::stringstream& header);

    /*
     * 所有属性对象 - marshal定义
     */
    void lang_gen_attribute_marshal_method_impls(std::stringstream& source);

    /*
     * 属性对象 - marshal定义
     */
    void lang_gen_attribute_marshal_method_impl(krpc_attribute_t* attribute, std::stringstream& source);

    /*
     * 所有属性对象 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_method_impls(std::stringstream& source);

    /*
     * 属性对象 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_method_impl(krpc_attribute_t* attribute, std::stringstream& source);

    /*
     * 属性对象 - 非数组 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_field_not_array(krpc_field_t* field, std::stringstream& source, const std::string& name, int index);

    /*
     * 属性对象 - 数组 - unmarshal定义
     */
    void lang_gen_attribute_unmarshal_field_array(krpc_field_t* field, std::stringstream& source, const std::string& name, int index);

    /*
     * 字段 - marshal定义
     */
    void lang_gen_field_marshal_impl(krpc_field_t* field, std::stringstream& source);

    /*
     * 字段 - 非数组 - marshal定义
     */
    void lang_gen_field_marshal_impl_not_array(krpc_field_t* field, std::stringstream& source, const std::string& holder_name = "",
        const std::string& vector_name = "", const std::string& whites = "");

    /*
     * 字段 - 数组 - marshal定义
     */
    void lang_gen_field_marshal_impl_array(krpc_field_t* field, std::stringstream& source, const std::string& whites = "");

    /*
     * 字段 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl(krpc_field_t* field, std::stringstream& source, int index);

    /*
     * 字段 - 非数组 - marshal定义
     */
    void lang_gen_field_unmarshal_impl_not_array(krpc_field_t* field, std::stringstream& source, int index);

    /*
     * 字段 - 非数组, 作为函数参数内调用 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl_not_array_inline(krpc_field_t* field, std::stringstream& source, int index);

    /*
     * 字段 - 数组 - unmarshal定义
     */
    void lang_gen_field_unmarshal_impl_array(krpc_field_t* field, std::stringstream& source, int index);

    /*
     * 取得属性字段类型名
     */
    std::string lang_field_find_type_name(krpc_field_t* field);

    /*
     * 取得参数字段类型名
     */
    std::string lang_param_find_type_name(krpc_field_t* field);

private:
    krpc_gen_t* _rpc_gen; // 代码生成入口类
    uint16_t    _rpc_id;  // RPC函数ID
};

#endif // KRPC_CPP_H
