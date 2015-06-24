#ifndef KRPC_H
#define KRPC_H

#include <map>
#include <list>

class krpc_parser_t;

typedef std::map<std::string, std::string> OptionMap;

class krpc_gen_t {
public:
    krpc_gen_t(int argc, char** argv);
    ~krpc_gen_t();
    void lang_gen_code();
    OptionMap& get_options();
    krpc_parser_t* get_parser() const;

private:
    bool krpc_gen_t::get_option(int pos, int argc, char** argv, const char* detail, const char* brief);

private:
    OptionMap      _options;
    int            _argc;
    char**         _argv;
    krpc_parser_t* _parser;
};

#endif // KRPC_H
