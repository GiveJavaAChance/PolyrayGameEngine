#ifndef BYTEWRITER_H_INCLUDED
#define BYTEWRITER_H_INCLUDED

#pragma once

#include <cstdint>
#include <structure/DynamicArray.h>

struct ByteReader {
    const uint8_t* data;
    size_t size;
    size_t pos;

    ByteReader(const uint8_t* data, size_t size) : data(data), size(size), pos(0ull) {
    }

    inline void read(void* dst, uint32_t size) {
        std::memcpy(dst, data + pos, size);
        pos += size;
    }

    template <typename T>
    inline void read(T& e) {
        constexpr uint32_t size = sizeof(T);
        std::memcpy(&e, data + pos, size);
        pos += size;
    }

    template <typename T>
    inline T read() {
        constexpr uint32_t size = sizeof(T);
        T e{};
        std::memcpy(&e, data + pos, size);
        pos += size;
        return e;
    }
};

struct ByteWriter {
    DynamicArray<uint8_t>* data;

    ByteWriter(DynamicArray<uint8_t>* d) : data(d) {
    }

    inline void write(void* d, uint32_t size) {
        data->addAll(reinterpret_cast<uint8_t*>(d), size);
    }

    template <typename T>
    inline void write(T& e) {
        data->addAll(reinterpret_cast<uint8_t*>(&e), sizeof(T));
    }

    template <typename T>
    inline void write(T&& e) {
        data->addAll(reinterpret_cast<uint8_t*>(&e), sizeof(T));
    }
};

#endif
