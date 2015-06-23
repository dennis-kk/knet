#ifndef KRPC_EXCEPTION_H
#define KRPC_EXCEPTION_H

#include <stdexcept>
#include <sstream>

#define raise_exception(param) \
    do { \
        std::stringstream ss; \
        ss << param; \
        throw std::logic_error(ss.str()); \
    } while(0);

#define check_raise_exception(b, param) \
    if (!(b)) { \
        raise_exception(param); \
    }

#endif /* KRPC_EXCEPTION_H */
