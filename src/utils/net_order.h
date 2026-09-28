#ifndef NET_ORDER_H_INCLUDED
#define NET_ORDER_H_INCLUDED

#pragma once

#include <cstdint>

template <typename T>
constexpr void reverse_bytes(T& v) noexcept {
    uint8_t* d = reinterpret_cast<uint8_t*>(&v);
    for (uint32_t i = 0u; i < sizeof(T) >> 1u; i++) {
        uint8_t tmp = d[i];
        d[i] = d[sizeof(T) - 1u - i];
        d[sizeof(T) - 1u - i] = tmp;
    }
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

template <typename T>
constexpr T hton(T v) noexcept {
    reverse_bytes(v);
    return v;
}

template <typename T>
constexpr T ntoh(T v) noexcept {
    reverse_bytes(v);
    return v;
}

#else

template <typename T>
constexpr T& hton(T& v) noexcept {
    return v;
}

template <typename T>
constexpr T& ntoh(T& v) noexcept {
    return v;
}

#endif

#endif