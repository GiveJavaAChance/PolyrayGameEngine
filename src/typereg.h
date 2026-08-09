#ifndef TYPEREG_H_INCLUDED
#define TYPEREG_H_INCLUDED

#pragma once

#include <cstdint>
#include <string_view>

#define TYPE_REGISTRY(name)             \
struct name {                           \
private:                                \
    inline static uint32_t nextID = 0;  \
public:                                 \
    template<typename T>                \
    static uint32_t getTypeId() {       \
        static uint32_t ID = nextID++;  \
        return ID;                      \
    }                                   \
};

template<typename T>
constexpr std::string_view type_name_of() {
    constexpr std::string_view func = __PRETTY_FUNCTION__;
    constexpr std::string_view prefix = "T = ";
    constexpr std::string_view suffix = ";";

    const size_t start = func.find(prefix) + prefix.size();
    const size_t end = func.rfind(suffix);
    return func.substr(start, end - start);
}

#endif
