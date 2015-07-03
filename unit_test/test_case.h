#ifndef TEST_CASE_H
#define TEST_CASE_H

#include <string>

class test_case_t {
public:
    test_case_t(const std::string& name) {
        _result = true;
        _name   = name;
    }

    virtual ~test_case_t() {
    }

    const std::string& get_name() {
        return _name;
    }

    void set_result(bool result) {
        _result = result;
    }

    bool get_result() {
        return _result;
    }

    void set_error(const std::string& error) {
        _error = error;
    }

    const std::string& get_error() {
        return _error;
    }

    void run() {
        run_case();
    }

private:
    virtual void run_case() = 0;

private:
    std::string _name;
    std::string _error;
    bool        _result;
};

#endif // TEST_CASE_H
