#include <iostream>

#include "testing.h"
#include "test_case.h"
#include "helper.h"
#include "all_test_case.h"

testing_t* testing_t::_instance = NULL;

testing_t::testing_t() {
    _current = NULL;
}

void testing_t::startup() {
}

void testing_t::cleanup() {
}

testing_t* testing_t::instance() {
    if (NULL == _instance) {
        _instance = new testing_t();
    }
    return _instance;
}

testing_t::~testing_t() {
    case_list_t::iterator guard = _case_list.begin();
    for (; guard != _case_list.end(); guard++) {
        delete *guard;
    }
    _case_list.clear();
}

void testing_t::show_all_test_cases() {
    case_list_t::iterator guard = _case_list.begin();
    std::cout << std::endl;
    for (; guard != _case_list.end(); guard++) {
        std::cout << blue << "[CASE]" << white << (*guard)->get_name() << std::endl;
    }
    std::cout << "Total " << _case_list.size() << " case" << std::endl;
}

void testing_t::start_all_test_cases() {
    case_list_t::iterator guard = _case_list.begin();
    int pass = 0;
    for (; guard != _case_list.end(); guard++) {
        _current = *guard;
        try {
            std::cout << std::endl << blue << "[RUN]" << white << _current->get_name() << std::endl;
            _current->run();
        } catch (std::exception& e) {
            _current->set_result(false);
            _current->set_error(e.what());
        }
        if (_current->get_result()) {
            std::cout << blue << "[PASS]" << white << _current->get_name() << std::endl;
            pass += 1;
        } else {
            std::cout << red << "[FAIL]" << white << _current->get_name() << std::endl;
            std::cout << red << "[FAIL]" << white << _current->get_error() << std::endl;
        }
    }

    std::cout << "Total " << _case_list.size() << " case" << std::endl;
    std::cout << blue << pass << " case pass" << std::endl;
    if (pass != (int)_case_list.size()) {
        std::cout << red << (int)_case_list.size() - pass << " case failed" << std::endl;
        guard = _case_list.begin();
        for (; guard != _case_list.end(); guard++) {
            if (!(*guard)->get_result()) {
                std::cout << red << "[FAIL]" << white << (*guard)->get_name() << std::endl;
                std::cout << red << "[ERROR]" << white << (*guard)->get_error() << std::endl;
            }
        }
    }
    std::cout << white;
}

test_case_t* testing_t::current() {
    return _current;
}

bool testing_t::add_case(test_case_t* testCase) {
    _case_list.push_back(testCase);
    return true;
}

int testing_t::size() {
    return (int)_case_list.size();
}
