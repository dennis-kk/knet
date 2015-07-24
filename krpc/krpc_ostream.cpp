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

#include <cstdarg>
#include "krpc_exception.h"
#include "krpc_ostream.h"

krpc_ostream_t::krpc_ostream_t(const std::string& file_name)
: _row(0) {
    _ofs.open(file_name.c_str());
    if (!_ofs) {
        raise_exception("open " << file_name << " failed");
    }
}

krpc_ostream_t::~krpc_ostream_t() {
    _ofs.close();
}

int krpc_ostream_t::analyze() {
    for (; *_stream; ) {
        if (want_skip('{')) { // 找到 '{'
            if (!want_skip('{')) { // 临接字符不是 '{'
                _ofs << '{';
                save();
            } else { // 临接字符是 '{'
                break;
            }
        } else {
            save();
        }
    }
    if (!*_stream) {
        return NONE;
    }
    int type = NONE;
    switch (*_stream++) {
    case '@':
        type = STRING;
        break;
    case '$':
        type = INTEGER;
        break;
    default:
        raise_exception(_file_name << "(" << _row
            << "):invalid template format, need a template type:\n"
            << _start);
    }
    for (; if_not('}'); ) {
        skip();
    }
    if (!want_skip('}') || !want_skip('}')) {
        raise_exception(_file_name << "(" << _row
            << "):invalid template format, need a '}':\n"
            << _start);
    }
    return type;
}

void krpc_ostream_t::save() {
    if (!*_stream) {
        return;
    }
    if (*_stream == '\n') {
        _row++;
    }
    _ofs << *_stream++;
}

bool krpc_ostream_t::if_not(char c) {
    if (!*_stream) {
        return false;
    }
    return (*_stream != c);
}

bool krpc_ostream_t::want(char c) {
    if (!*_stream) {
        return false;
    }
    return (*_stream && (*_stream == c));
}

bool krpc_ostream_t::want_skip(char c) {
    if (!want(c)) {
        return false;
    }
    _stream++;
    return true;
}

void krpc_ostream_t::skip() {
    if (!*_stream) {
        return;
    }
    _stream++;
}

const krpc_ostream_t& krpc_ostream_t::write(const char* fmt, ...) {
    _row = 1;
    _file_name = "";
    va_list argv;
    va_start(argv, fmt);
    _stream = const_cast<char*>(fmt);
    _start = _stream;
    for (; *_stream; ) {
        switch (analyze()) {
        case INTEGER:
            _ofs << va_arg(argv, int);
            break;
        case STRING:
            _ofs << va_arg(argv, char*);
            break;
        }
    }
    va_end(argv);
    return *this;
}

const krpc_ostream_t& krpc_ostream_t::replace(const char* fmt,
    const std::string& source) {
    _row = 1;
    _file_name = "";
    _stream = const_cast<char*>(fmt);
    _start = _stream;
    for (; *_stream; ) {
        switch (analyze()) {
        case INTEGER:
            raise_exception("string only");
            break;
        case STRING:
            _ofs << source;
            break;
        }
    }
    return *this;
}

const krpc_ostream_t& krpc_ostream_t::write_template(const char* file_name, ...) {
    _row = 1;
    _file_name = file_name;
    std::string line;
    read_file(file_name, line);
    _stream = const_cast<char*>(line.c_str());
    _start = _stream;
    va_list argv;
    va_start(argv, file_name);
    for (; *_stream; ) {
        switch (analyze()) {
        case INTEGER:
            _ofs << va_arg(argv, int);
            break;
        case STRING:
            _ofs << va_arg(argv, char*);
            break;
        }
    }
    va_end(argv);
    return *this;
}

const krpc_ostream_t& krpc_ostream_t::replace_template(const char* file_name, const std::string& source) {
    _row = 1;
    _file_name = file_name;
    std::string line;
    read_file(file_name, line);
    _stream = const_cast<char*>(line.c_str());
    _start = _stream;
    for (; *_stream; ) {
        switch (analyze()) {
        case INTEGER:
            raise_exception("string only");
            break;
        case STRING:
            _ofs << source;
            break;
        }
    }
    return *this;
}

void krpc_ostream_t::read_file(const char* file_name, std::string& source) {
    _row = 1;
    _file_name = file_name;
    std::ifstream ifs;
    ifs.open(file_name);
    if (!ifs) {
        raise_exception("open " << file_name << " failed");
    }
    while (!ifs.eof()) {
        char c = 0;
        ifs.read(&c, 1);
        source += c;
    }
}
