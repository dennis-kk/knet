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
            std::cout << std::endl << yellow << "[RUN]" << white << _current->get_name() << std::endl;
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
