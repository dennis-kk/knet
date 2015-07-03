#ifndef TESTING_H
#define TESTING_H

#include <list>

class test_case_t;

class testing_t {
public:
    testing_t();
    ~testing_t();

    static void startup();
    static void cleanup();
    static testing_t* instance();

    void show_all_test_cases();
    void start_all_test_cases();
    bool add_case(test_case_t* testCase);
    int size();
    test_case_t* current();

private:
    typedef std::list<test_case_t*> case_list_t;
    case_list_t       _case_list;
    static testing_t* _instance;
    test_case_t*      _current;
};

#endif // TESTING_H
