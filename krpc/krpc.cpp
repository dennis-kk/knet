#include <iostream>
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
    _parser = new krpc_parser_t(0, _options["dir"].c_str(), _options["file"].c_str());
    _parser->parse();
}

bool krpc_gen_t::get_option(int pos, int argc, char** argv, const char* detail, const char* brief) {
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

OptionMap& krpc_gen_t::get_options() {
    return _options;
}

krpc_parser_t* krpc_gen_t::get_parser() const {
    return _parser;
}

void krpc_gen_t::lang_gen_code() {
    if (_options["lang"] == "cpp") {
        krpc_gen_cpp_t gen_cpp(this);
        gen_cpp.lang_gen_code();
    } else {
        std::cout << "the language '" << _options["lang"] << "'" << " is not supported" << std::endl;
        exit(0);
    }
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
