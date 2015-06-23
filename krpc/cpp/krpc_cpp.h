#ifndef KRPC_CPP_H
#define KRPC_CPP_H

#include "krpc.h"

class krpc_gen_t;

class krpc_gen_cpp_t {
public:
    krpc_gen_cpp_t(krpc_gen_t* rpc_gen);
    ~krpc_gen_cpp_t();
    void lang_gen_code();

private:
    krpc_gen_t* _rpc_gen;
};

#endif // KRPC_CPP_H
