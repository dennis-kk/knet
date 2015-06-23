#include "krpc.h"
#include "krpc_parser.h"
#include "krpc_cpp.h"

krpc_gen_t::krpc_gen_t(int argc, char** argv) {
    // TODO ½âÎöÃüÁîÐÐ
    _options["lang"] = "cpp";
    _options["file"] = "e:\\test.h";
    _parser = new krpc_parser_t(0, _options["file"].c_str());
    _parser->parse();
}

krpc_gen_t::~krpc_gen_t() {
    if (_parser) {
        delete _parser;
    }
}

const OptionMap& krpc_gen_t::getOptions() const {
    return _options;
}

krpc_parser_t* krpc_gen_t::get_parser() const {
    return _parser;
}

void krpc_gen_t::lang_gen_code() {
    if (_options["lang"] == "cpp") {
        krpc_gen_cpp_t gen_cpp(this);
        gen_cpp.lang_gen_code();
    }
}

int main(int argc, char** argv) {
    krpc_gen_t rpc_gen(argc, argv);
    rpc_gen.lang_gen_code();
    return 0;
}
