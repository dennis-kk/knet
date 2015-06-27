#ifndef KRPC_OSTREAM_H
#define KRPC_OSTREAM_H

#include <iostream>
#include <fstream>
#include <sstream>

/*
 * 标准文件输出类包装器
 */
class krpc_ostream_t {
public:
    /*
     * 构造函数
     * @param file_name 需要写入的文件路径
     */
    krpc_ostream_t(const std::string& file_name);

    /*
     * 析构
     */
    ~krpc_ostream_t();

    /*
     * std::string
     */
    krpc_ostream_t& operator<<(const std::string& s) {
        _ofs << s;
        return *this;
    }

    krpc_ostream_t& operator<<(int i) {
        std::stringstream ss;
        ss << i;
        _ofs << ss.str();
        return *this;
    }

    /*
     * const char*
     */
    krpc_ostream_t& operator<<(const char* s) {
        _ofs << s;
        return *this;
    }
    
    /*
     * 可变参数模板替换
     */
    const krpc_ostream_t& write(const char* fmt, ...);
    const krpc_ostream_t& replace(const char* fmt, const std::string& source);

private:
    std::ofstream _ofs; // 文件输出流
};

#endif // KRPC_OSTREAM_H
