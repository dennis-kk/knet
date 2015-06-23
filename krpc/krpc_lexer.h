#ifndef KRPC_LEXER_H
#define KRPC_LEXER_H

#include <exception>
#include <string>

class krpc_token_t;
class krpc_lexer_t;

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
    krpc_token_comma,
};

class krpc_token_t {
public:
    krpc_token_t(const std::string& literal, int row, int col, int type);
    ~krpc_token_t();
    const std::string& get_literal() const;
    int get_row() const;
    int get_col() const;
    int get_type() const;

private:
    std::string _literal;
    int _row;
    int _col;
    int _type;
};

class krpc_lexer_t {
public:
    krpc_lexer_t(const char* stream);
    ~krpc_lexer_t();
    krpc_token_t* next_token() throw(std::exception);
    int get_col();
    int get_row();

private:
    krpc_token_t* get_token();
    void eat_whites();
    void eat_comment();
    bool check_white(char c);
    bool check_terminator(char c);
    char forward(int step);
    void add_col(int cols);
    void add_row();
    bool verify();
    char current();
    bool has_next();
    bool check_var_name(const char* var_name);
    bool check_keyword2(const char* s, char c1, char c2);
    bool check_keyword3(const char* s, char c1, char c2, char c3);
    bool check_keyword4(const char* s, char c1, char c2, char c3, char c4);
    bool check_keyword5(const char* s, char c1, char c2, char c3, char c4, char c5);
    bool check_keyword6(const char* s, char c1, char c2, char c3, char c4, char c5, char c6);

private:
    int   _row;
    int   _col;
    char* _stream;
};

#endif // KRPC_LEXER_H
