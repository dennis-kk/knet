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

#ifndef KRPC_H
#define KRPC_H

#include <map>
#include <list>

class krpc_parser_t;

/*
 * rpc框架代码生成类
 */
class krpc_gen_t {
public:
    typedef std::map<std::string, std::string> option_map_t;

public:
    /*
     * 构造函数
     * @param argc 命令行个数
     * @param argv 命令行
     */
    krpc_gen_t(int argc, char** argv);

    /*
     * 析构函数
     */
    ~krpc_gen_t();

    /*
     * 生成代码
     */
    void lang_gen_code();

    /*
     * 获取参数表
     * @return 获取参数表
     */
    option_map_t& get_options();

    /*
     * 获取语法解析器
     * @return krpc_parser_t实例
     */
    krpc_parser_t* get_parser() const;

private:
    /*
     * 获取命令行参数
     * @param pos 命令行数组位置
     * @param argc 命令行个数
     * @param argv 命令行
     * @param detail 完整参数
     * @param brief 简写
     * @retval true 找到参数
     * @retval false 未找到参数
     */
    bool get_option(int pos, int argc, char** argv, const char* detail, const char* brief);

private:
    option_map_t   _options; // 参数表
    int            _argc;    // 命令行个数
    char**         _argv;    // 命令行
    krpc_parser_t* _parser;  // 语法解析器
};

#endif // KRPC_H
