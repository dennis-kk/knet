#ifndef KRPC_PARSER_H
#define KRPC_PARSER_H

#include <list>
#include <map>
#include <exception>
#include <string>
#include "krpc_lexer.h"

class krpc_field_t;

enum {
    krpc_field_type_i8 = 1,
    krpc_field_type_i16 =2,
    krpc_field_type_i32 = 4,
    krpc_field_type_i64 = 8,
    krpc_field_type_ui8 = 16,
    krpc_field_type_ui16 = 32,
    krpc_field_type_ui32 = 64,
    krpc_field_type_ui64 = 128,
    krpc_field_type_f32 = 256,
    krpc_field_type_f64 = 512,
    krpc_field_type_string = 1024,
    krpc_field_type_array = 2048,
    krpc_field_type_attribute = 4096,
    krpc_field_type_rpc = 8192,
};

class krpc_attribute_t {
public:
    typedef std::list<krpc_field_t*> FieldList;
    typedef FieldList::iterator iterator;

public:
    krpc_attribute_t(const std::string& name);
    ~krpc_attribute_t();
    void push_field(krpc_field_t* field);
    const std::string& get_name();
    FieldList& get_field_list();

private:
    FieldList _fields;
    std::string _name;
};

class krpc_field_t {
public:
    krpc_field_t(int type);
    ~krpc_field_t();
    void set_type(int type);
    bool check_array();
    bool check_type(int type);
    void set_field_name(const std::string& field_name);
    void set_field_type(const std::string& field_type);
    const std::string& get_field_name();
    const std::string& get_field_type();

private:
    int convert(int type);

private:
    std::string _field_name;
    std::string _field_type;
    int _type;
};

class krpc_rpc_call_t {
public:
    krpc_rpc_call_t(const std::string& rpc_name);
    ~krpc_rpc_call_t();
    const std::string& get_name();
    krpc_attribute_t* get_attribute();

private:
    std::string _name;
    krpc_attribute_t* _attribute;
};

class krpc_parser_t {
public:
    typedef std::map<std::string, krpc_attribute_t*> ObjectMap;
    typedef std::map<std::string, krpc_rpc_call_t*>  RpcCallMap;

public:
    krpc_parser_t(krpc_parser_t* parent, const char* dir, const char* file_name);
    ~krpc_parser_t();
    void parse() throw(std::exception);
    ObjectMap& get_attributes();
    RpcCallMap& get_rpc_calls();
    void print();

private:
    krpc_token_t* next_token();
    void parse_trunk(krpc_token_t* token);
    krpc_attribute_t* parse_attribute(krpc_token_t* token);
    krpc_rpc_call_t* parse_rpc_call(krpc_token_t* token);
    void parse_import(krpc_token_t* token);
    krpc_field_t* parse_field(krpc_token_t* token);

private:
    ObjectMap      _objects;
    RpcCallMap     _rpc_calls;
    std::string    _file_name;
    std::string    _dir;
    krpc_lexer_t*  _lexer;
    krpc_parser_t* _parent;
    krpc_parser_t* _current_parser;
};

#endif // KRPC_PARSER_H
