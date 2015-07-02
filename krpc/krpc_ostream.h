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

#ifndef KRPC_OSTREAM_H
#define KRPC_OSTREAM_H

#include <iostream>
#include <fstream>
#include <sstream>

/**
 * 标准文件输出类包装器
 */
class krpc_ostream_t {
public:
    /**
     * 构造函数
     * @param file_name 需要写入的文件路径
     */
    krpc_ostream_t(const std::string& file_name);

    /**
     * 析构
     */
    ~krpc_ostream_t();

    /**
     * std::string
     * @param s std::string
     * @return krpc_ostream_t引用
     */
    krpc_ostream_t& operator<<(const std::string& s) {
        _ofs << s;
        return *this;
    }

    /**
     * int
     * @param i int
     * @return krpc_ostream_t引用
     */
    krpc_ostream_t& operator<<(int i) {
        std::stringstream ss;
        ss << i;
        _ofs << ss.str();
        return *this;
    }

    /**
     * const char*
     * @param s c string
     * @return krpc_ostream_t引用
     */
    krpc_ostream_t& operator<<(const char* s) {
        _ofs << s;
        return *this;
    }
    
    /**
     * 可变参数模板替换，只支持字符串参数
     * @param fmt 格式
     * @return krpc_ostream_t引用
     */
    const krpc_ostream_t& write(const char* fmt, ...);

    /**
     * 替换单个模板
     * @param fmt 格式
     * @param source 源字符串
     * @return krpc_ostream_t引用
     */
    const krpc_ostream_t& replace(const char* fmt, const std::string& source);

    /**
     * 从文件内读取模板，可变参数模板替换，只支持字符串参数
     * @param file_name 路径
     * @return krpc_ostream_t引用
     */
    const krpc_ostream_t& write_template(const char* file_name, ...);

    /**
     * 从文件内读取模板，并替换单个模板
     * @param file_name 路径
     * @param source 源字符串
     * @return krpc_ostream_t引用
     */
    const krpc_ostream_t& replace_template(const char* file_name, const std::string& source);

private:
    /**
     * 模板类型
     */
    enum {
        NONE,    // 非模板
        INTEGER, // 整数$
        STRING,  // 字符串@
    };

    /**
     * 分析并提取模板
     * @return 模板类型
     */
    int analyze();

    /**
     * 检查下一个字符是否为指定字符
     * @param c 字符
     * @retval true 是
     * @retval false 不是
     */
    bool want(char c);

    /**
     * 检查下一个字符是否为指定字符并跳过
     * @param c 字符
     * @retval true 是
     * @retval false 不是
     */
    bool want_skip(char c);

    /**
     * 检查下一个字符是否为非指定字符
     * @param c 字符
     * @retval true 是
     * @retval false 不是
     */
    bool if_not(char c);

    /**
     * 跳过一个字符
     */
    void skip();

    /**
     * 输出一个字符
     */
    void save();

    /**
     * 文件读取
     * @param file_name 路径
     * @param source 读到的内容
     */
    void read_file(const char* file_name, std::string& source);

private:
    int           _row;       ///< 当前行
    std::string   _file_name; ///< 当前模板
    std::ofstream _ofs;       ///< 文件输出流
    char*         _stream;    ///< 输入字符流
    char*         _start;     ///< 当前输入流起始
};

#endif // KRPC_OSTREAM_H
