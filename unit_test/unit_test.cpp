#include "helper.h"

int main() {
    testing_t::startup();
    testing_t::instance()->start_all_test_cases();
    testing_t::cleanup();
    return 0;
}
