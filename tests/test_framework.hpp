#ifndef TEST_FRAMEWORK_HPP
#define TEST_FRAMEWORK_HPP

// NOLINTNEXTLINE(misc-include-cleaner) - required for std::cout in RUN_TEST macro
#include <iostream>

#define RUN_TEST(name) do { \
    std::cout << "  " #name "... "; \
    name(); \
    std::cout << "PASSED\n"; \
} while(0)

#endif
