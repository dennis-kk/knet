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

#include <cassert>
#include <cstdarg>
#include <cctype>
#include "krpc_lexer.h"
#include "krpc_exception.h"

krpc_token_t::krpc_token_t(const std::string& literal, int row, int col, int type)
: _literal(literal),
  _row(row),
  _col(col),
  _type(type) {
}

krpc_token_t::~krpc_token_t() {
}

const std::string& krpc_token_t::get_literal() const {
    return _literal;
}

int krpc_token_t::get_row() const {
    return _row;
}

int krpc_token_t::get_col() const {
    return _col;
}

int krpc_token_t::get_type() const {
    return _type;
}

krpc_lexer_t::krpc_lexer_t(const char* stream)
: _row(1),
  _col(1),
  _stream(const_cast<char*>(stream)) {
}

krpc_lexer_t::~krpc_lexer_t() {
    token_list_t::iterator guard = _tokens.begin();
    for (; guard != _tokens.end(); guard++) {
        delete *guard;
    }
}

krpc_token_t* krpc_lexer_t::next_token() throw(std::exception) {
    krpc_token_t* token = 0;
    if (current()) {
        token = get_token();
    }
    if (token) {
        _tokens.push_back(token);
    }
    return token;
}

krpc_token_t* krpc_lexer_t::get_token() {
    static const int MAX_TOKEN_SIZE = 64;
    char token[MAX_TOKEN_SIZE] = {0};
    int  i         = 0;
    int  type      = 0;
    char c         = 0;
    if (!verify()) {
        return 0;
    }
    eat_whites();
    for (c = current(); (c) && (!check_terminator(c)); c = forward(1), i++) {
        token[i] = c;
    }
    if (i >= MAX_TOKEN_SIZE) {
        raise_exception("reach max name length");
    }
    if (!token[0] && check_terminator(c)) {
        token[i++] = current();
    }
    if (!token[0]) {
        return 0;
    }
    switch (token[0]) {
    case 'o':
        if (check_keyword6(token, 'o', 'b', 'j', 'e', 'c', 't')) {
            type = krpc_token_object;
            add_col(6);
        }
        break;
    case 'i':
        if (check_keyword2(token, 'i', '8')) {
            type = krpc_token_i8;
            add_col(2);
        } else if (check_keyword3(token, 'i', '1', '6')) {
            type = krpc_token_i16;
            add_col(3);
        } else if (check_keyword3(token, 'i', '3', '2')) {
            type = krpc_token_i32;
            add_col(3);
        } else if (check_keyword3(token, 'i', '6', '4')) {
            type = krpc_token_i64;
            add_col(3);
        } else if (check_keyword6(token, 'i', 'm', 'p', 'o', 'r', 't')) {
            type = krpc_token_import;
            add_col(6);
        }
        break;
    case 'u':
        if (check_keyword3(token, 'u', 'i', '8')) {
            type = krpc_token_ui8;
            add_col(3);
        } else if (check_keyword4(token, 'u', 'i', '1', '6')) {
            type = krpc_token_ui16;
            add_col(4);
        } else if (check_keyword4(token, 'u', 'i', '3', '2')) {
            type = krpc_token_ui32;
            add_col(4);
        } else if (check_keyword4(token, 'u', 'i', '6', '4')) {
            type = krpc_token_ui64;
            add_col(4);
        }
        break;
    case 'f':
        if (check_keyword3(token, 'f', '3', '2')) {
            type = krpc_token_f32;
            add_col(3);
        } else if (check_keyword3(token, 'f', '6', '4')) {
            type = krpc_token_f64;
            add_col(3);
        }
        break;
    case 's':
        if (check_keyword6(token, 's', 't', 'r', 'i', 'n', 'g')) {
            type = krpc_token_string;
            add_col(6);
        }
        break;
    case '{':
        forward(1);
        add_col(1);
        type = krpc_token_left_brace;
        break;
    case '}':
        forward(1);
        add_col(1);
        type = krpc_token_right_brace;
        break;
    case '(':
        forward(1);
        add_col(1);
        type = krpc_token_left_round;
        break;
    case ')':
        forward(1);
        add_col(1);
        type = krpc_token_right_round;
        break;
    case 'r':
        if (check_keyword3(token, 'r', 'p', 'c')) {
            type = krpc_token_rpc;
            add_col(3);
        }
        break;
    case ',':
        forward(1);
        add_col(1);
        type = krpc_token_comma;
        break;
    case '[':
        if (has_next() && check_keyword2(_stream, '[', ']')) {
            // 数组
            forward(2);
            add_col(2);
            type = krpc_token_array;
        } else {
            raise_exception("invalid array declaration");
        }
        break;
    case '/':
        // 单行/多行注释
        forward(1);
        eat_comment();
        return get_token();
    default:
        break;
    }
    if (0 == type) {
        // 变量名，对象名
        add_col(i);
        if (!check_var_name(token)) {
            raise_exception("variable name must start by '_' or letter");
        }
        type = krpc_token_text;
    }
    return new krpc_token_t(token, _row, _col, type);
}

bool krpc_lexer_t::check_var_name(const char* var_name) {
    if ((var_name[0] != '_') && !isalpha(var_name[0])) {
        return false;
    }
    return true;
}

void krpc_lexer_t::add_col(int cols) {
    _col += cols;
}

void krpc_lexer_t::inc_row() {
    _row++;
}

int krpc_lexer_t::get_col() {
    return _col;
}

int krpc_lexer_t::get_row() {
    return _row;
}

char krpc_lexer_t::forward(int step) {
    _stream += step;
    return *_stream;
}

char krpc_lexer_t::current() {
    return *_stream;
}

bool krpc_lexer_t::has_next() {
    return (0 != _stream[1]);
}

void krpc_lexer_t::eat_whites() {
    char c = 0;
    for (c = current(); (c); c = forward(1)) {
        switch (c) {
            case ' ':
            case '\r':
                add_col(1);
                break;
            case '\t':
                add_col(4);
                break;
            case '\n':
                _col = 1;
                inc_row();
                break;
            default:
                return;
        }
    }
}

bool krpc_lexer_t::check_white(char c) {
    switch (c) {
    case ' ': case '\t': case '\r': case '\n':
        return true;
    }
    return false;
}

bool krpc_lexer_t::check_terminator(char c) {
    switch (c) {
    case '{': case '}': case '(': case ')': case ',': case '[': case '/':
        return true;
    }
    return check_white(c);
}

bool krpc_lexer_t::check_keyword2(const char* s, char c1, char c2) {
    return (s[0] == c1) && (s[1] == c2);
}

bool krpc_lexer_t::check_keyword3(const char* s, char c1, char c2, char c3) {
    return (s[0] == c1) && (s[1] == c2) && (s[2] == c3);
}

bool krpc_lexer_t::check_keyword4(const char* s, char c1, char c2, char c3, char c4) {
    return (s[0] == c1) && (s[1] == c2) && (s[2] == c3) && (s[3] == c4);
}

bool krpc_lexer_t::check_keyword5(const char* s, char c1, char c2, char c3, char c4, char c5) {
    return (s[0] == c1) && (s[1] == c2) && (s[2] == c3) && (s[3] == c4) && (s[4] == c5);
}

bool krpc_lexer_t::check_keyword6(const char* s, char c1, char c2, char c3, char c4, char c5, char c6) {
    return (s[0] == c1) && (s[1] == c2) && (s[2] == c3) && (s[3] == c4) && (s[4] == c5) && (s[5] == c6);
}

bool krpc_lexer_t::verify() {
    return (_stream && _stream[0]);
}

void krpc_lexer_t::eat_comment() {
    check_raise_exception(verify(), "uncomplete comment");
    if (current() == '/') {
        // 单行注释
        for (; verify(); forward(1), add_col(1)) {
            if (current() == '\n') {
                _col = 1;
                inc_row();
                forward(1);
                return;
            }
        }
    } else {
        raise_exception("invalid comment");
    }
    eat_whites();
}
