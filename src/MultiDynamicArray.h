#ifndef MULTIDYNAMICARRAY_H_INCLUDED
#define MULTIDYNAMICARRAY_H_INCLUDED

#pragma once

#include <cstdint>
#include <cstring>
#include <immintrin.h>

using u32 = uint32_t;

template<typename... Ts>
struct MultiDynamicArray {
private:
    template<u32 I>
    using ColumnType = std::tuple_element_t<I, std::tuple<Ts...>>;

    u32 length;
    std::tuple<Ts*...> arrays;
    void* ptr;
    u32 pos;

    template<std::size_t... Is>
    inline std::tuple<Ts*...> makeTupleFromOffsets(void* block, u32* offsets, std::index_sequence<Is...>) {
        return std::make_tuple(reinterpret_cast<Ts*>(reinterpret_cast<uint8_t*>(block) + offsets[Is])...);
    }

    inline std::tuple<Ts*...> allocate(u32 len) {
        u32 offsets[sizeof...(Ts)] = {};
        u32 idx = 0;
        u32 totalSize = 0u;
        ((offsets[idx] = totalSize, totalSize += (len * sizeof(Ts) + 31u) & (~31u), idx++), ...);
        void* block = _mm_malloc(totalSize, 32);
        idx = 0;
        return makeTupleFromOffsets(block, offsets, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<std::size_t... Is>
    inline void ensureCapacityImpl(std::tuple<Ts*...>& newArrays, std::index_sequence<Is...>) {
        ((std::memcpy(std::get<Is>(newArrays),std::get<Is>(arrays), pos * sizeof(ColumnType<Is>))), ...);
    }

    template<typename Tuple, std::size_t... Is>
    inline void addImpl(u32 i, Tuple&& vals, std::index_sequence<Is...>) {
        ((std::get<Is>(arrays)[i] = std::get<Is>(std::forward<Tuple>(vals))), ...);
    }

    template<std::size_t... Is>
    inline void setImpl(const u32 dst, const MultiDynamicArray& other, const u32 src, std::index_sequence<Is...>) {
        ((std::get<Is>(arrays)[dst] = std::get<Is>(other.arrays)[src]), ...);
    }

    template<std::size_t... Is>
    inline void clearImpl(std::index_sequence<Is...>) {
        ((std::memset(std::get<Is>(arrays), 0, pos * sizeof(ColumnType<Is>))), ...);
    }

public:

    MultiDynamicArray(u32 initialLength = 16u) : length(initialLength), arrays(allocate(initialLength)), ptr(std::get<0>(arrays)), pos(0u) {
    }

    ~MultiDynamicArray() {
        _mm_free(ptr);
    }

    void ensureCapacity(u32 cap) {
        if (cap <= length) {
            return;
        }
        u32 newLength = (cap * 3u) >> 1u;
        std::tuple<Ts*...> newArrays = allocate(newLength);
        ensureCapacityImpl(newArrays, std::make_index_sequence<sizeof...(Ts)>{});
        _mm_free(ptr);
        arrays = newArrays;
        ptr = std::get<0>(arrays);
        length = newLength;
    }

    template<typename... Us>
    void add(Us&&... values) {
        static_assert(sizeof...(Ts) == sizeof...(Us));
        ensureCapacity(pos + 1u);
        addImpl(pos++, std::forward_as_tuple(std::forward<Us>(values)...), std::make_index_sequence<sizeof...(Ts)>{});
    }

    void set(const u32 dstIdx, const MultiDynamicArray& other, const u32 srcIdx) {
        setImpl(dstIdx, other, srcIdx, std::make_index_sequence<sizeof...(Ts)>{});
    }

    template<typename... Us>
    void setIndex(const u32 dstIdx, Us&&... values) {
        static_assert(sizeof...(Ts) == sizeof...(Us));
        ensureCapacity(dstIdx + 1u);
        addImpl(dstIdx, std::forward_as_tuple(std::forward<Us>(values)...), std::make_index_sequence<sizeof...(Ts)>{});
        if(dstIdx >= pos) {
            pos = dstIdx + 1u;
        }
    }

    inline void reserve(u32 count) {
        ensureCapacity(pos + count);
        pos += count;
    }

    inline void removeEnd(u32 count = 1u) {
        if(count > pos) {
            pos = 0;
        } else {
            pos -= count;
        }
    }

    void clear(bool zero = false) {
        if (zero) {
            clearImpl(std::make_index_sequence<sizeof...(Ts)>{});
        }
        pos = 0;
    }

    inline u32 size() const {
        return pos;
    }

    template<u32 i>
    inline typename std::tuple_element<i, std::tuple<Ts...>>::type* column() {
        return std::get<i>(arrays);
    }
};

#endif
