#ifndef PACKETREADER_H_INCLUDED
#define PACKETREADER_H_INCLUDED

#pragma once

#include <bit>
#include <cstdint>

#include <utils/net_order.h>

struct PacketReader {
private:
    void* data;
    uint32_t pos;

public:
    PacketReader(void* data) : data(data), pos(0u) {
    }

    template <typename T>
    inline T get() {
        T v{};
        std::memcpy(&v, data, sizeof(T));
        pos += sizeof(T);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        reverse_bytes<T>(v);
#endif
        return v;
    }

    template <typename T>
    inline void getAll(T* v, uint32_t count) {
        std::memcpy(&v, data, sizeof(T) * count);
        pos += sizeof(T) * count;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        for (uint32_t i = 0u; i < count; i++) {
            reverse_bytes<T>(v[i]);
        }
#endif
    }
};

#endif
