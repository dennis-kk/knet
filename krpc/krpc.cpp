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

#include <iostream>
#include <stdlib.h>
#include "krpc.h"
#include "krpc_parser.h"
#include "krpc_cpp.h"

krpc_gen_t::krpc_gen_t(int argc, char** argv) {
    // 简单的解析命令行
    for (int pos = 1; pos < argc; pos++) {
        if (get_option(pos, argc, argv, "lang", "l") ||
            get_option(pos, argc, argv, "file", "f") ||
            get_option(pos, argc, argv, "dir",  "d") ||
            get_option(pos, argc, argv, "name", "n")) {
            pos++;
        } 
    }
    // 开始解析协议文件
    _parser = new krpc_parser_t(0, _options["dir"].c_str(),
        _options["file"].c_str());
    _parser->parse();
}

bool krpc_gen_t::get_option(int pos, int argc, char** argv, const char* detail,
    const char* brief) {
    std::string detail_option = "-";
    std::string brief_option = "-";
    detail_option += detail;
    brief_option += brief;
    if ((argv[pos] == detail_option) || (argv[pos] == brief_option)) {
        if (pos + 1 < argc) {
            _options[detail] = argv[pos + 1];
            return true;
        } else {
            std::cout << "need specify '" << detail << "'" << std::endl;
            exit(0);
        }
    }
    return false;
}

krpc_gen_t::~krpc_gen_t() {
    if (_parser) {
        delete _parser;
    }
}

krpc_gen_t::option_map_t& krpc_gen_t::get_options() {
    return _options;
}

krpc_parser_t* krpc_gen_t::get_parser() const {
    return _parser;
}

void krpc_gen_t::lang_gen_code() {
    if (_options["lang"] == "cpp") { // c++生成
        krpc_gen_cpp_t gen_cpp(this);
        gen_cpp.gen_code();
    } else {
        std::cout << "the language '" << _options["lang"] << "'"
            << " is not supported" << std::endl;
    }
}

const std::string& krpc_gen_t::get_option(const std::string& key) {
    return _options[key];
}

int main(int argc, char** argv) {
    try {
        krpc_gen_t rpc_gen(argc, argv);
        rpc_gen.lang_gen_code();
    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    return 0;
}
