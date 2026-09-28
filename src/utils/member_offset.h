#ifndef MEMBER_OFFSET_H_INCLUDED
#define MEMBER_OFFSET_H_INCLUDED

#pragma once

#include <cstdint>

template <typename T, typename V, V T::* Member>
constexpr uint32_t member_offset() {
    return static_cast<uint32_t>((uintptr_t) (&(((T*) nullptr)->*Member)));
}

#endif