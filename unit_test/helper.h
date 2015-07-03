#ifndef HELPER_H
#define HELPER_H

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
        testing_t::instance()->current()->set_result(false); \
        testing_t::instance()->current()->set_error(#expr); \
    }

#define EXPECT_FALSE(expr) \
    EXPECT_TRUE(!(expr))

#include <ostream>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

inline static std::ostream& blue(std::ostream &s) {
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
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

#else

#include <cstdio>

inline static std::ostream& blue(std::ostream &s) {
    printf("\033[1;34m");
    return s;
}

inline static std::ostream& red(std::ostream &s) {
    printf("\033[1;31m");
    return s;
}

inline static std::ostream& green(std::ostream &s) {
    printf("\033[1;32m");
    return s;
}

inline static std::ostream& yellow(std::ostream &s) {
    printf("\033[1;33m");
    return s;
}

inline static std::ostream& white(std::ostream &s) {
    printf("\033[30m");
    return s;
}

#endif // WIN32

#include "testing.h"
#include "test_case.h"

#endif // HEPLER_H
