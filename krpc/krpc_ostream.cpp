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
            fmt++; // skip @
            for (; *fmt && (*fmt != '@'); ) {
                fmt++;
            }
            if (*fmt) {
                fmt++; // skip @
            } else {
                break;
            }
            _ofs << va_arg(argv, char*);
        }
    }
    va_end(argv);
    return *this;
}

const krpc_ostream_t& krpc_ostream_t::replace(const char* fmt,
    const std::string& source) {
    for (; *fmt; ) {
        if (*fmt != '@') {
            _ofs.write(fmt++, 1);
        } else {
            fmt++; // skip @
            for (; *fmt && (*fmt != '@'); ) {
                fmt++;
            }
            if (*fmt) {
                fmt++; // skip @
            } else {
                break;
            }
            _ofs << source;
        }
    }
    return *this;
}
