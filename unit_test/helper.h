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
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef HELPER_H
#define HELPER_H

#include <cstdio>
#include <sstream>

#define CASE(name) \
    class TestCase_##name : public test_case_t { \
    public: \
        TestCase_##name(const std::string& caseName) : test_case_t(caseName) {} \
        virtual ~TestCase_##name() {} \
    private: \
        virtual void run_case(); \
        static bool _holder; \
    }; \
    bool TestCase_##name::_holder = testing_t::instance()->add_case(new TestCase_##name(#name)); \
    \
    void TestCase_##name::run_case()

#define EXPECT_TRUE(expr) \
    if (!(expr)) { \
        std::stringstream ss; \
        ss << "(" << __FILE__ << ":" << __LINE__ << ")" << #expr; \
        testing_t::instance()->current()->set_result(false); \
        testing_t::instance()->current()->set_error(ss.str()); \
    }

#define EXPECT_TRUE_OUTPUT(expr, output) \
    if (!(expr)) { \
        std::stringstream ss; \
        ss << "(" << __FILE__ << ":" << __LINE__ << ")" << #expr << "," << output; \
        testing_t::instance()->current()->set_result(false); \
        testing_t::instance()->current()->set_error(ss.str()); \
    }

#define EXPECT_FALSE(expr) \
    EXPECT_TRUE(!(expr))

#define EXPECT_FALSE_OUTPUT(expr, output) \
    EXPECT_TRUE_OUTPUT(!(expr), output)

#define CASE_FAIL() \
    testing_t::instance()->current()->set_result(false);

#include <ostream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

inline static std::ostream& blue(std::ostream &s) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    return s;
}

inline static std::ostream& red(std::ostream &s) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
    return s;
}

inline static std::ostream& green(std::ostream &s) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    return s;
}

inline static std::ostream& yellow(std::ostream &s) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    return s;
}

inline static std::ostream& white(std::ostream &s) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    return s;
}

inline std::string getBinaryPath() {
    char path[MAX_PATH] = {0};
    GetModuleFileNameA(0, path, sizeof(path));
    int i = MAX_PATH - 1;
    for (; (path[i] != '\\') && (path[i] != '/'); i--);
    path[i] = 0;
    return path;
}

#else

#include <cstdio>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>

inline static std::ostream& blue(std::ostream &s) {
    //printf("\033[1;34m");
    return s;
}

inline static std::ostream& red(std::ostream &s) {
    //printf("\033[1;31m");
    return s;
}

inline static std::ostream& green(std::ostream &s) {
    //printf("\033[1;32m");
    return s;
}

inline static std::ostream& yellow(std::ostream &s) {
    //printf("\033[1;33m");
    return s;
}

inline static std::ostream& white(std::ostream &s) {
    //printf("\033[30m");
    return s;
}

inline std::string getBinaryPath() {
    char link[PATH_MAX] = {0};
    char path[PATH_MAX] = {0};
    sprintf(link, "/proc/%d/exe", getpid());
    readlink(link, path, sizeof(path));
    int i = PATH_MAX - 1;
    for (; (path[i] != '\\') && (path[i] != '/'); i--);
    path[i] = 0;
    return path;
}

#endif // WIN32

#include "testing.h"
#include "test_case.h"

#include "config.h"
#if defined(USE_IPV6)
#define LOOP_ADDR "0:0:0:0:0:0:0:1"
#define LOCAL_ADDR "0:0:0:0:0:0:0:0"
#else
#define LOOP_ADDR "127.0.0.1"
#define LOCAL_ADDR "0.0.0.0"
#endif

#endif // HEPLER_H
