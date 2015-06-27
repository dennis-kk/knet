#include <cstdarg>
#include "krpc_exception.h"
#include "krpc_ostream.h"

krpc_ostream_t::krpc_ostream_t(const std::string& file_name) {
    _ofs.open(file_name.c_str());
    if (!_ofs) {
        raise_exception("open " << file_name << " failed");
    }
}

krpc_ostream_t::~krpc_ostream_t() {
    _ofs.close();
}

const krpc_ostream_t& krpc_ostream_t::write(const char* fmt, ...) {
    va_list argv;
    va_start(argv, fmt);
    for (; *fmt; ) {
        if (*fmt != '@') {
            _ofs.write(fmt++, 1);
        } else {
            fmt++;
            for (; *fmt && (*fmt != '@'); ) {
                fmt++;
            }
            if (!*fmt) {
                break;
            } else {
                fmt++;
            }
            _ofs << va_arg(argv, char*);
        }
    }
    va_end(argv);
    return *this;
}

const krpc_ostream_t& krpc_ostream_t::replace(const char* fmt, const std::string& source) {
    for (; *fmt; ) {
        if (*fmt != '@') {
            _ofs.write(fmt++, 1);
        } else {
            fmt++; // skip @
            for (; *fmt && (*fmt != '@'); ) {
                fmt++;
            }
            if (!*fmt) {
                break;
            } else {
                fmt++;
            }
            _ofs << source;
        }
    }
    return *this;
}
