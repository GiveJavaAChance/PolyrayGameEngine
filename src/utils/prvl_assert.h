#ifndef PRVL_ASSERT_H_INCLUDED
#define PRVL_ASSERT_H_INCLUDED

#pragma once

#include <iostream>
#include <source_location>

inline void assertFailed(const char* condition, std::string_view message, std::source_location location = std::source_location::current()) {
    std::cerr << "Assertion failed: " << condition
              << "\nMessage: " << message
              << "\nFile: " << location.file_name()
              << "\nLine: " << location.line()
              << "\nFunction: " << location.function_name() << std::endl;
    std::exit(1);
}

#ifdef NDEBUG
#define PRVL_ASSERT(condition, message) ((void) 0)
#else
#define PRVL_ASSERT(condition, message)    \
    if (!(condition)) {                    \
        assertFailed(#condition, message); \
    }
#endif

#endif