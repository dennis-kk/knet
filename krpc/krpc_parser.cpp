#include <fstream>
#include <sstream>
#include <iostream>
#include "krpc_parser.h"
#include "krpc_exception.h"

krpc_field_t::krpc_field_t(int type)
: _type(convert(type)) {
}

krpc_field_t::~krpc_field_t() {
}

void krpc_field_t::set_type(int type) {
    _type |= type;
}

bool krpc_field_t::check_array() {
    return (krpc_field_type_array == (_type & krpc_field_type_array));
}

const std::string& krpc_field_t::get_field_name() {
    return _field_name;
}

bool krpc_field_t::check_type(int type) {
    return (type == (type & _type));
}

int krpc_field_t::convert(int type) {
    switch (type) {
    case krpc_token_text:
        return krpc_field_type_attribute;
    case krpc_token_i8:
        return krpc_field_type_i8;
    case krpc_token_i16:
        return krpc_field_type_i16;
    case krpc_token_i32:
        return krpc_field_type_i32;
    case krpc_token_i64:
        return krpc_field_type_i64;
    case krpc_token_ui8:
        return krpc_field_type_ui8;
    case krpc_token_ui16:
        return krpc_field_type_ui16;
    case krpc_token_ui32:
        return krpc_field_type_ui32;
    case krpc_token_ui64:
        return krpc_field_type_ui64;
    case krpc_token_f32:
        return krpc_field_type_f32;
    case krpc_token_f64:
        return krpc_field_type_f64;
    case krpc_token_string:
        return krpc_field_type_string;
    case krpc_token_array:
        return krpc_field_type_array;
    }
    return 0;
}

void krpc_field_t::set_field_name(const std::string& field_name) {
    _field_name = field_name;
}

void krpc_field_t::set_field_type(const std::string& field_type) {
    _field_type = field_type;
}

const std::string& krpc_field_t::get_field_type() {
    return _field_type;
}

krpc_attribute_t::krpc_attribute_t(const std::string& name)
: _name(name) {
}

krpc_attribute_t::~krpc_attribute_t() {
    // 清理
    FieldList::iterator guard = _fields.begin();
    for (; guard != _fields.end(); guard++) {
        delete *guard;
    }
}

void krpc_attribute_t::push_field(krpc_field_t* field) {
    // 检查重复
    FieldList::iterator guard = _fields.begin();
    for (; guard != _fields.end(); guard++) {
        if (field->get_field_name() == (*guard)->get_field_name()) {
            raise_exception("field redefined '" << field->get_field_name() << "'");
        }
    }
    _fields.push_back(field);
}

const std::string& krpc_attribute_t::get_name() {
    return _name;
}

krpc_attribute_t::FieldList& krpc_attribute_t::get_field_list() {
    return _fields;
}

krpc_rpc_call_t::krpc_rpc_call_t(const std::string& rpc_name)
: _name(rpc_name),
  _attribute(new krpc_attribute_t("")) {
}

krpc_rpc_call_t::~krpc_rpc_call_t() {
    delete _attribute;
}

const std::string& krpc_rpc_call_t::get_name() {
    return _name;
}

krpc_attribute_t* krpc_rpc_call_t::get_attribute() {
    return _attribute;
}

krpc_parser_t::krpc_parser_t(krpc_parser_t* parent, const char* dir, const char* file_name)
: _parent(parent), 
  _file_name(file_name),
  _dir(dir),
  _lexer(0),
  _current_parser(0) {
}

krpc_parser_t::~krpc_parser_t() {
    ObjectMap::iterator object = _objects.begin();
    for (; object != _objects.end(); object++) {
        delete object->second;
    }
    RpcCallMap::iterator rpc_call = _rpc_calls.begin();
    for (; rpc_call != _rpc_calls.end(); rpc_call++) {
        delete rpc_call->second;
    }
    if (_lexer) {
        delete _lexer;
    }
}

krpc_token_t* krpc_parser_t::next_token() {
    return _lexer->next_token();
}

krpc_field_t* krpc_parser_t::parse_field(krpc_token_t* token) {
    //
    // `type` `name`
    //
    check_raise_exception(token, "need a field type");
    krpc_token_t* first = token;
    krpc_field_t* field = new krpc_field_t(first->get_type());
    field->set_field_type(first->get_literal());
    check_raise_exception(token = next_token(), "need a field type or array declaration");
    if (krpc_token_array == token->get_type()) {
        field->set_type(krpc_field_type_array);
        check_raise_exception(token = next_token(), "need a field name");
    }
    check_raise_exception(krpc_token_text == token->get_type(), "need a field name but got a '" << token->get_literal() << "'");
    field->set_field_name(token->get_literal());
    return field;
}

krpc_attribute_t* krpc_parser_t::parse_attribute(krpc_token_t* token) {
    //
    // attribute `name` {
    //     `field`
    //     ...
    // }
    //
    krpc_attribute_t* attribute = new krpc_attribute_t(token->get_literal());
    check_raise_exception(token = next_token(), "attribute need a '{'");
    check_raise_exception(krpc_token_left_brace == token->get_type(), "need a '{'");
    for (token = next_token(); token && (krpc_token_right_brace != token->get_type()); token = next_token()) {
        krpc_field_t* field = parse_field(token);
        attribute->push_field(field);
    }
    check_raise_exception(krpc_token_right_brace == token->get_type(), "need a '}'");
    return attribute;
}

krpc_rpc_call_t* krpc_parser_t::parse_rpc_call(krpc_token_t* token) {
    //
    // rpc `name` ( `field` , ... )
    //
    check_raise_exception(token, "attribute need a RPC name");
    check_raise_exception(krpc_token_text == token->get_type(), "attribute need a RPC name");
    krpc_rpc_call_t* rpc_call = new krpc_rpc_call_t(token->get_literal());
    check_raise_exception(token = next_token(), "need a '('");
    check_raise_exception(krpc_token_left_round == token->get_type(), "need a '('");
    for (token = next_token(); token; token = next_token()) {
        krpc_field_t* field = parse_field(token);
        rpc_call->get_attribute()->push_field(field);
        check_raise_exception(token = next_token(), "attribute need a ',' or ')'");
        check_raise_exception((krpc_token_comma == token->get_type()) || (krpc_token_right_round == token->get_type()), "attribute need a ',' or ')'");
        if (krpc_token_right_round == token->get_type()) {
            break;
        }
    }
    return rpc_call;
}

void krpc_parser_t::parse_import(krpc_token_t* token) {
    //
    // import `file`
    //
    check_raise_exception(token, "need a file name");
    krpc_parser_t* parser = new krpc_parser_t(this, _dir.c_str(), token->get_literal().c_str());
    _current_parser = parser;
    parser->parse();
    _current_parser = this;
    delete parser;
}

void krpc_parser_t::parse_trunk(krpc_token_t* token) {
    //
    // `object-declare` | `rpc-call` | `single-line-comment` | `import-file`
    //
    if (krpc_token_object == token->get_type()) { // 对象
        krpc_attribute_t* attribute = parse_attribute(next_token());
        // 检查重复
        if (get_attributes().find(attribute->get_name()) != get_attributes().end()) {
            raise_exception("object redefined '" << attribute->get_name() << "'");
        }
        get_attributes().insert(std::make_pair(attribute->get_name(), attribute));
    } else if (krpc_token_rpc == token->get_type()) { // RPC方法
        krpc_rpc_call_t* rpc_call = parse_rpc_call(next_token());
        // 检查重复
        if (get_rpc_calls().find(rpc_call->get_name()) != get_rpc_calls().end()) {
            raise_exception("rpc method redefined '" << rpc_call->get_name() << "'");
        }
        get_rpc_calls().insert(std::make_pair(rpc_call->get_name(), rpc_call));
    } else if (krpc_token_import == token->get_type()) { // 文件
        // 导入其他文件
        parse_import(next_token());
    } else {
        raise_exception("invalid trunk format");
    }
}

void krpc_parser_t::parse() throw(std::exception) {
    std::ifstream ifs;
    std::string file_name = _dir;
    file_name += _file_name;
    ifs.open(file_name.c_str());
    if (!ifs) {
        raise_exception("open " << file_name << " failed");
    }
    std::string source;
    while (!ifs.eof()) {
        char c = 0;
        ifs.read(&c, 1);
        source += c;
    }
    _lexer = new krpc_lexer_t(source.c_str());
    _current_parser = this;
    try {
        krpc_token_t* token = 0;
        for (; token = next_token(); ) {
            parse_trunk(token);
        }
    } catch (std::exception& e) {
        // 错误输出, 文件(行，列): 原因
        std::cout << _current_parser->_file_name << "(" << _lexer->get_row() << "," << _lexer->get_col() << "): " << e.what() << std::endl;
    }
}

krpc_parser_t::ObjectMap& krpc_parser_t::get_attributes() {
    // 如果有上层parser，全部导入上层
    if (_parent) {
        return _parent->get_attributes();
    }
    return _objects;
}

krpc_parser_t::RpcCallMap& krpc_parser_t::get_rpc_calls() {
    // 如果有上层parser，全部导入上层
    if (_parent) {
        return _parent->get_rpc_calls();
    }
    return _rpc_calls;
}

void krpc_parser_t::print() {
}
