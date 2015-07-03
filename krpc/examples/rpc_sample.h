//
// KRPC - Generated code, *DO NOT CHANGE*
//

#ifndef _krpc_rpc_sample_h_
#define _krpc_rpc_sample_h_

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include "knet.h"

namespace rpc_sample {

struct my_object_other_t;
struct my_object_t;

/**
 * 自定义对象
 */
struct my_object_other_t {
	std::map<std::string,std::string> string_string_table; ///< 表
    
	/**
     * 构造函数
     */
    my_object_other_t();

    /**
     * 拷贝构造
     * \param rht my_object_other_t引用
     */
    my_object_other_t(const my_object_other_t& rht);

    /**
     * 赋值
     * \param rht my_object_other_t引用
     */
    const my_object_other_t& operator=(const my_object_other_t& rht);

    /**
     * 打印对象
     * \param ss std::stringstream引用， 对象信息将输出到
     * \param white 缩进空格
     */
    void print(std::stringstream& ss, std::string white = "");
};

/**
 * 对象事例
 */
struct my_object_t {
	int8_t ni8; ///< 有符号8位整数
	int16_t ni16; ///< 有符号16位整数
	int32_t ni32; ///< 有符号32位整数
	int64_t ni64; ///< 有符号64位整数
	uint8_t nui8; ///< 无符号8位整数
	uint16_t nui16; ///< 无符号16位整数
	uint32_t nui32; ///< 无符号32位整数
	uint64_t nui64; ///< 无符号64位整数
	float nf32; ///< 32位浮点数
	double nf64; ///< 64位浮点数
	std::string str; ///< 字符串
	std::map<int8_t,std::string> int_string_table; ///< 表
	std::map<int8_t,my_object_other_t> int_object_table; ///< 表
    
	/**
     * 构造函数
     */
    my_object_t();

    /**
     * 拷贝构造
     * \param rht my_object_t引用
     */
    my_object_t(const my_object_t& rht);

    /**
     * 赋值
     * \param rht my_object_t引用
     */
    const my_object_t& operator=(const my_object_t& rht);

    /**
     * 打印对象
     * \param ss std::stringstream引用， 对象信息将输出到
     * \param white 缩进空格
     */
    void print(std::stringstream& ss, std::string white = "");
};

/**
 * my_object_other_t序列化
 */
krpc_object_t* marshal(my_object_other_t& o);

/**
 * my_object_other_t反序列化
 */
bool unmarshal(krpc_object_t* v, my_object_other_t& o);

/**
 * my_object_t序列化
 */
krpc_object_t* marshal(my_object_t& o);

/**
 * my_object_t反序列化
 */
bool unmarshal(krpc_object_t* v, my_object_t& o);

/**
 * my_rpc_func1代理
 */
krpc_object_t* my_rpc_func1_proxy(my_object_t& my_obj);

/**
 * my_rpc_func2代理
 */
krpc_object_t* my_rpc_func2_proxy(std::vector<my_object_t>& my_objs, int8_t my_i8);

/**
 * my_rpc_func3代理
 */
krpc_object_t* my_rpc_func3_proxy(const std::string& my_str, int8_t my_i8);

/**
 * my_rpc_func1桩
 */
int my_rpc_func1_stub(krpc_object_t* o);

/**
 * my_rpc_func2桩
 */
int my_rpc_func2_stub(krpc_object_t* o);

/**
 * my_rpc_func3桩
 */
int my_rpc_func3_stub(krpc_object_t* o);

/**
 * RPC调用示例, my_rpc_func1声明，需实现此方法
 * \param my_obj 参数1
 * \retval rpc_ok          成功
 * \retval rpc_close       忽略错误，关闭
 * \retval rpc_error       错误，但不关闭
 * \retval rpc_error_close 错误且关闭
 */
int my_rpc_func1(my_object_t& my_obj);

/**
 * RPC调用示例, my_rpc_func2声明，需实现此方法
 * \param my_objs 参数1
 * \param my_i8 参数2
 * \retval rpc_ok          成功
 * \retval rpc_close       忽略错误，关闭
 * \retval rpc_error       错误，但不关闭
 * \retval rpc_error_close 错误且关闭
 */
int my_rpc_func2(std::vector<my_object_t>& my_objs, int8_t my_i8);

/**
 * RPC调用示例, my_rpc_func3声明，需实现此方法
 * \param my_str 参数1
 * \param my_i8 参数2
 * \retval rpc_ok          成功
 * \retval rpc_close       忽略错误，关闭
 * \retval rpc_error       错误，但不关闭
 * \retval rpc_error_close 错误且关闭
 */
int my_rpc_func3(const std::string& my_str, int8_t my_i8);

/**
 * RPC单件类
 */
class rpc_sample_t {
public:
    /**
     * 析构
     */
    ~rpc_sample_t();

    /**
     * 取得单件指针
     * \return rpc_sample_t指针
     */
    static rpc_sample_t* instance();

    /**
     * 销毁单件
     */
    static void finalize();

    /**
     * 从stream_t读取RPC调用请求
     * \param stream stream_t实例
     * \retval error_ok 成功处理一次RPC调用
     * \retval error_rpc_not_enough_bytes 没有完整的RPC可以处理
     * \retval error_rpc_unmarshal_fail 处理RPC包字节流时读取失败
     * \retval error_rpc_unknown_id 读取到RPC调用，但RPC ID未注册
     * \retval error_rpc_cb_fail 调用RPC处理函数时，处理函数内部发生错误
     * \retval error_rpc_cb_fail_close 调用RPC处理函数时，处理函数内部发生错误，处理函数要求关闭stream_t相关联的管道
     * \retval error_rpc_cb_close 调用RPC处理函数后，处理函数要求关闭stream_t相关联的管道
     * \retval error_rpc_unknown_type RPC类型错误
     */
    int rpc_proc(stream_t* stream);

    /**
     * my_rpc_func1 RPC调用示例
	 * \param stream stream_t实例
    * \param my_obj 参数1
    * \retval error_ok 成功
    * \retval error_rpc_marshal_fail 序列化RPC调用时失败
    */
	int my_rpc_func1(stream_t* stream, my_object_t& my_obj);

    /**
     * my_rpc_func2 RPC调用示例
	 * \param stream stream_t实例
    * \param my_objs 参数1
    * \param my_i8 参数2
    * \retval error_ok 成功
    * \retval error_rpc_marshal_fail 序列化RPC调用时失败
    */
	int my_rpc_func2(stream_t* stream, std::vector<my_object_t>& my_objs, int8_t my_i8);

    /**
     * my_rpc_func3 RPC调用示例
	 * \param stream stream_t实例
    * \param my_str 参数1
    * \param my_i8 参数2
    * \retval error_ok 成功
    * \retval error_rpc_marshal_fail 序列化RPC调用时失败
    */
	int my_rpc_func3(stream_t* stream, const std::string& my_str, int8_t my_i8);

private:
    /**
     * 构造函数
     */
    rpc_sample_t();

    /**
     * 拷贝构造
     */
    rpc_sample_t(const rpc_sample_t&);

private:
    static rpc_sample_t* _instance; // 单件指针
    krpc_t* _rpc; // RPC实现类"
};

/**
 * rpc_sample鍗曚欢璁块棶甯姪鍑芥暟
 */
inline static rpc_sample_t* rpc_sample_ptr() {
	return rpc_sample_t::instance();
}

} // namespace rpc_sample

#endif // _krpc_rpc_sample_h_

