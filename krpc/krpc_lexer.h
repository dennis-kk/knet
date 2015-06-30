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

#ifndef KRPC_LEXER_H
#define KRPC_LEXER_H

#include <exception>
#include <string>
#include <list>

class krpc_token_t;
class krpc_lexer_t;

/**
 * token类型
 */
enum {
    krpc_token_i8 = 1,
    krpc_token_i16,
    krpc_token_i32,
    krpc_token_i64,
    krpc_token_ui8,
    krpc_token_ui16,
    krpc_token_ui32,
    krpc_token_ui64,
    krpc_token_f32,
    krpc_token_f64,
    krpc_token_string,
    krpc_token_array,
    krpc_token_object,
    krpc_token_rpc,
    krpc_token_import,
    krpc_token_text,
    krpc_token_left_brace,
    krpc_token_right_brace,
    krpc_token_left_round,
    krpc_token_right_round,
    krpc_token_right_square,
    krpc_token_comma,
    krpc_token_inline_comment,
};

/**
 * token
 */
class krpc_token_t {
public:
    /**
     * 构造函数
     * @param literal 字面量
     * @param row 行
     * @param col 列
     * @param type 类型
     */
    krpc_token_t(const std::string& literal, int row, int col, int type);

    /**
     * 析构
     */
    ~krpc_token_t();

    /**
     * 取得token字面量
     * @return token字面量
     */
    const std::string& get_literal() const;

    /**
     * 取得行
     */
    int get_row() const;

    /**
     * 取得列
     */
    int get_col() const;

    /**
     * 取得类型
     */
    int get_type() const;

private:
    std::string _literal; // 字面量
    int         _row;     // 行
    int         _col;     // 列
    int         _type;    // 类型
};

/**
 * token解析器
 */
class krpc_lexer_t {
public:
    /**
     * 构造
     * @param stream 字符流
     */
    krpc_lexer_t(const char* stream);

    /**
     * 析构
     */
    ~krpc_lexer_t();

    /**
     * 取得下一个token
     * @return krpc_token_t实例
     */
    krpc_token_t* next_token() throw(std::exception);

    /**
     * 取得当前行
     */
    int get_col();

    /**
     * 取得当前列
     */
    int get_row();

private:
    /**
     * 取得下一个token
     * @return krpc_token_t实例
     */
    krpc_token_t* get_token();

    /**
     * 消除流当前空白
     */
    void eat_whites();

    /**
     * 消除流当前注释
     */
    void eat_comment();

    /**
     * 检查是否为空白
     * @param c 当前字符
     * @retval true 是
     * @retval false 否
     */
    bool check_white(char c);

    /**
     * 检查是否为终结符
     * @param c 当前字符
     * @param icomment 是否在嵌入注释体内
     * @retval true 是
     * @retval false 否
     */
    bool check_terminator(char c, bool icomment);

    /**
     * 步进流
     * @param step 步长
     * @return 当前字符
     */
    char forward(int step);

    /**
     * 增加列
     * @param cols 列
     */
    void add_col(int cols);

    /**
     * 递增行
     */
    void inc_row();

    /**
     * 检查流是否有效
     */
    bool verify();

    /**
     * 取得当前字符
     */
    char current();

    /**
     * 流是否结束
     */
    bool has_next();

    /*
     * 取下一个字符但不递增字符流
     */
    char try_next();

    /**
     * 检查变量名是否合法
     * @param var_name 变量名
     * @retval true 是
     * @retval false 否
     */
    bool check_var_name(const char* var_name);

    //
    // 关键字比较
    //

    bool check_keyword(const char* s, char c1, char c2);
    bool check_keyword(const char* s, char c1, char c2, char c3);
    bool check_keyword(const char* s, char c1, char c2, char c3, char c4);
    bool check_keyword(const char* s, char c1, char c2, char c3, char c4, char c5);
    bool check_keyword(const char* s, char c1, char c2, char c3, char c4, char c5, char c6);

private:
    typedef std::list<krpc_token_t*> token_list_t;
    token_list_t _tokens;   // token链表
    int          _row;      // 当前行
    int          _col;      // 当前列
    char*        _stream;   // 字符流
    bool         _icomment; // 是否在嵌入注释体内
};

#endif // KRPC_LEXER_H
