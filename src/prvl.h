#ifndef PRVL_H_INCLUDED
#define PRVL_H_INCLUDED

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>

#define _prvl_VEC2(type, name)                                                  \
struct name##2 {                                                                \
    union {                                                                     \
        struct {                                                                \
            type x;                                                             \
            type y;                                                             \
        };                                                                      \
        struct {                                                                \
            type r;                                                             \
            type g;                                                             \
        };                                                                      \
        struct {                                                                \
            type s;                                                             \
            type t;                                                             \
        };                                                                      \
    };                                                                          \
    constexpr name##2() noexcept : x{}, y{} {}                                  \
    constexpr name##2(const type x, const type y) noexcept;                     \
    constexpr name##2(const name##3& v) noexcept;                               \
    constexpr name##2(const name##4& v) noexcept;                               \
    explicit constexpr name##2(const type v) noexcept;                          \
    constexpr inline type& operator[](const uint32_t idx) noexcept {            \
        return *(&x + idx);                                                     \
    }                                                                           \
    constexpr inline const type& operator[](const uint32_t idx) const noexcept {\
        return *(&x + idx);                                                     \
    }                                                                           \
};

#define _prvl_VEC3(type, name)                                                  \
struct name##3 {                                                                \
    union {                                                                     \
        struct {                                                                \
            type x;                                                             \
            type y;                                                             \
            type z;                                                             \
        };                                                                      \
        struct {                                                                \
            type r;                                                             \
            type g;                                                             \
            type b;                                                             \
        };                                                                      \
        struct {                                                                \
            type s;                                                             \
            type t;                                                             \
            type p;                                                             \
        };                                                                      \
    };                                                                          \
    constexpr name##3() noexcept : x{}, y{}, z{} {}                             \
    constexpr name##3(const type x, const type y, const type z) noexcept;       \
    constexpr name##3(const name##2& xy, const type z) noexcept;                \
    constexpr name##3(const type x, const name##2& yz) noexcept;                \
    constexpr name##3(const name##4& v) noexcept;                               \
    explicit constexpr name##3(const type v) noexcept;                          \
    constexpr inline type& operator[](const uint32_t idx) noexcept {            \
        return *(&x + idx);                                                     \
    }                                                                           \
    constexpr inline const type& operator[](const uint32_t idx) const noexcept {\
        return *(&x + idx);                                                     \
    }                                                                           \
};

#define _prvl_VEC4(type, name)                                                          \
struct name##4 {                                                                        \
    union {                                                                             \
        struct {                                                                        \
            type x;                                                                     \
            type y;                                                                     \
            type z;                                                                     \
            type w;                                                                     \
        };                                                                              \
        struct {                                                                        \
            type r;                                                                     \
            type g;                                                                     \
            type b;                                                                     \
            type a;                                                                     \
        };                                                                              \
        struct {                                                                        \
            type s;                                                                     \
            type t;                                                                     \
            type p;                                                                     \
            type q;                                                                     \
        };                                                                              \
    };                                                                                  \
    constexpr name##4() noexcept : x{}, y{}, z{}, w{} {}                                \
    constexpr name##4(const type x, const type y, const type z, const type w) noexcept; \
    constexpr name##4(const name##2& xy, const type z, const type w) noexcept;          \
    constexpr name##4(const type x, const name##2& yz, const type w) noexcept;          \
    constexpr name##4(const type x, const type y, const name##2& zw) noexcept;          \
    constexpr name##4(const name##2& xy, const name##2& zw) noexcept;                   \
    constexpr name##4(const name##3& xyz, const type w) noexcept;                       \
    constexpr name##4(const type x, const name##3& yzw) noexcept;                       \
    explicit constexpr name##4(const type v) noexcept;                                  \
    constexpr inline type& operator[](const uint32_t idx) noexcept {                    \
        return *(&x + idx);                                                             \
    }                                                                                   \
    constexpr inline const type& operator[](const uint32_t idx) const noexcept {        \
        return *(&x + idx);                                                             \
    }                                                                                   \
};


#define _prvl_VEC_ALL(type, name)   \
struct name##2;                     \
struct name##3;                     \
struct name##4;                     \
_prvl_VEC2(type, name)              \
_prvl_VEC3(type, name)              \
_prvl_VEC4(type, name)

#define _prvl_IMPL_VEC2(type, name)                                             \
constexpr name##2::name##2(const type x, const type y) noexcept : x(x), y(y) {} \
constexpr name##2::name##2(const name##3& v) noexcept : x(v.x), y(v.y) {}       \
constexpr name##2::name##2(const name##4& v) noexcept : x(v.x), y(v.y) {}       \
constexpr name##2::name##2(const type v) noexcept : x(v), y(v) {}

#define _prvl_IMPL_VEC3(type, name)                                                                 \
constexpr name##3::name##3(const type x, const type y, const type z) noexcept : x(x), y(y), z(z) {} \
constexpr name##3::name##3(const name##2& xy, const type z) noexcept : x(xy.x), y(xy.y), z(z) {}    \
constexpr name##3::name##3(const type x, const name##2& yz) noexcept : x(x), y(yz.x), z(yz.y) {}    \
constexpr name##3::name##3(const name##4& v) noexcept : x(v.x), y(v.y), z(v.z) {}                   \
constexpr name##3::name##3(const type v) noexcept : x(v), y(v), z(v) {}

#define _prvl_IMPL_VEC4(type, name)                                                                                     \
constexpr name##4::name##4(const type x, const type y, const type z, const type w) noexcept : x(x), y(y), z(z), w(w) {} \
constexpr name##4::name##4(const name##2& xy, const type z, const type w) noexcept : x(xy.x), y(xy.y), z(z), w(w) {}    \
constexpr name##4::name##4(const type x, const name##2& yz, const type w) noexcept : x(x), y(yz.x), z(yz.y), w(w) {}    \
constexpr name##4::name##4(const type x, const type y, const name##2& zw) noexcept : x(x), y(y), z(zw.x), w(zw.y) {}    \
constexpr name##4::name##4(const name##2& xy, const name##2& zw) noexcept : x(xy.x), y(xy.y), z(zw.x), w(zw.y) {}       \
constexpr name##4::name##4(const name##3& xyz, const type w) noexcept : x(xyz.x), y(xyz.y), z(xyz.z), w(w) {}           \
constexpr name##4::name##4(const type x, const name##3& yzw) noexcept : x(x), y(yzw.x), z(yzw.y), w(yzw.z) {}           \
constexpr name##4::name##4(const type v) noexcept : x(v), y(v), z(v), w(v) {}

#define _prvl_IMPL_ALL(type, name)  \
_prvl_IMPL_VEC2(type, name)         \
_prvl_IMPL_VEC3(type, name)         \
_prvl_IMPL_VEC4(type, name)

#define _prvl_CAST2(nameA, nameB, typeA)                            \
constexpr inline static nameA _##nameA(const nameB& v) noexcept {   \
    return nameA(static_cast<typeA>(v.x), static_cast<typeA>(v.y)); \
}

#define _prvl_CAST3(nameA, nameB, typeA)                                                    \
constexpr inline static nameA _##nameA(const nameB& v) noexcept {                           \
    return nameA(static_cast<typeA>(v.x), static_cast<typeA>(v.y), static_cast<typeA>(v.z));\
}

#define _prvl_CAST4(nameA, nameB, typeA)                                                                                \
constexpr inline static nameA _##nameA(const nameB& v) noexcept {                                                       \
    return nameA(static_cast<typeA>(v.x), static_cast<typeA>(v.y), static_cast<typeA>(v.z), static_cast<typeA>(v.w));   \
}

#define _prvl_CAST_ALL(to, from, type)  \
_prvl_CAST2(to##2, from##2, type)       \
_prvl_CAST3(to##3, from##3, type)       \
_prvl_CAST4(to##4, from##4, type)

#define _prvl_VEC_BINARY_OP(name, op)                                                       \
constexpr inline static name##2 operator op(const name##2& a, const name##2& b) noexcept {  \
    return name##2(a.x op b.x, a.y op b.y);                                                 \
}                                                                                           \
constexpr inline static name##2& operator op##=(name##2& a, const name##2& b) noexcept {    \
    a.x op##= b.x;                                                                          \
    a.y op##= b.y;                                                                          \
    return a;                                                                               \
}                                                                                           \
constexpr inline static name##3 operator op(const name##3& a, const name##3& b) noexcept {  \
    return name##3(a.x op b.x, a.y op b.y, a.z op b.z);                                     \
}                                                                                           \
constexpr inline static name##3& operator op##=(name##3& a, const name##3& b) noexcept {    \
    a.x op##= b.x;                                                                          \
    a.y op##= b.y;                                                                          \
    a.z op##= b.z;                                                                          \
    return a;                                                                               \
}                                                                                           \
constexpr inline static name##4 operator op(const name##4& a, const name##4& b) noexcept {  \
    return name##4(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w);                         \
}                                                                                           \
constexpr inline static name##4& operator op##=(name##4& a, const name##4& b) noexcept {    \
    a.x op##= b.x;                                                                          \
    a.y op##= b.y;                                                                          \
    a.z op##= b.z;                                                                          \
    a.w op##= b.w;                                                                          \
    return a;                                                                               \
}

#define _prvl_VEC_UNARY_OP(name, op)                                    \
constexpr inline static name##2 operator op(const name##2& v) noexcept {\
    return name##2(op v.x, op v.y);                                     \
}                                                                       \
constexpr inline static name##3 operator op(const name##3& v) noexcept {\
    return name##3(op v.x, op v.y, op v.z);                             \
}                                                                       \
constexpr inline static name##4 operator op(const name##4& v) noexcept {\
    return name##4(op v.x, op v.y, op v.z, op v.w);                     \
}

#define _prvl_VEC_SCALAR_OP(type, name, op)                                             \
constexpr inline static name##2 operator op(const name##2& a, const type b) noexcept {  \
    return name##2(a.x op b, a.y op b);                                                 \
}                                                                                       \
constexpr inline static name##2& operator op##=(name##2& a, const type b) noexcept {    \
    a.x op##= b;                                                                        \
    a.y op##= b;                                                                        \
    return a;                                                                           \
}                                                                                       \
constexpr inline static name##2 operator op(const type a, const name##2& b) noexcept {  \
    return name##2(a op b.x, a op b.y);                                                 \
}                                                                                       \
constexpr inline static name##3 operator op(const name##3& a, const type b) noexcept {  \
    return name##3(a.x op b, a.y op b, a.z op b);                                       \
}                                                                                       \
constexpr inline static name##3& operator op##=(name##3& a, const type b) noexcept {    \
    a.x op##= b;                                                                        \
    a.y op##= b;                                                                        \
    a.z op##= b;                                                                        \
    return a;                                                                           \
}                                                                                       \
constexpr inline static name##3 operator op(const type a, const name##3& b) noexcept {  \
    return name##3(a op b.x, a op b.y, a op b.z);                                       \
}                                                                                       \
constexpr inline static name##4 operator op(const name##4& a, const type b) noexcept {  \
    return name##4(a.x op b, a.y op b, a.z op b, a.w op b);                             \
}                                                                                       \
constexpr inline static name##4& operator op##=(name##4& a, const type b) noexcept {    \
    a.x op##= b;                                                                        \
    a.y op##= b;                                                                        \
    a.z op##= b;                                                                        \
    a.w op##= b;                                                                        \
    return a;                                                                           \
}                                                                                       \
constexpr inline static name##4 operator op(const type a, const name##4& b) noexcept {  \
    return name##4(a op b.x, a op b.y, a op b.z, a op b.w);                             \
}                                                                                       \

#define _prvl_VEC_CMP(name, cmp)                                                            \
constexpr inline static bvec2 operator cmp(const name##2& a, const name##2& b) noexcept {   \
    return bvec2(a.x cmp b.x, a.y cmp b.y);                                                 \
}                                                                                           \
constexpr inline static bvec3 operator cmp(const name##3& a, const name##3& b) noexcept {   \
    return bvec3(a.x cmp b.x, a.y cmp b.y, a.z cmp b.z);                                    \
}                                                                                           \
constexpr inline static bvec4 operator cmp(const name##4& a, const name##4& b) noexcept {   \
    return bvec4(a.x cmp b.x, a.y cmp b.y, a.z cmp b.z, a.w cmp b.w);                       \
}

#define _prvl_VEC_DOT(type, name)                                               \
constexpr inline static type dot(const name##2& a, const name##2& b) noexcept { \
    return a.x * b.x + a.y * b.y;                                               \
}                                                                               \
constexpr inline static type dot(const name##3& a, const name##3& b) noexcept { \
    return a.x * b.x + a.y * b.y + a.z * b.z;                                   \
}                                                                               \
constexpr inline static type dot(const name##4& a, const name##4& b) noexcept { \
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;                       \
}

#define _prvl_VEC_CROSS(name)                                                           \
constexpr inline static name##3 cross(const name##3& a, const name##3& b) noexcept {    \
    return name##3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);\
}

#define _prvl_VEC_LENGTH(type, name)                            \
constexpr inline static type length(const name##2& v) noexcept {\
    return sqrtf(v.x * v.x + v.y * v.y);                        \
}                                                               \
constexpr inline static type length(const name##3& v) noexcept {\
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);            \
}                                                               \
constexpr inline static type length(const name##4& v) noexcept {\
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);\
}

#define _prvl_VEC_NORMALIZE(name)                                       \
constexpr inline static name##2 normalize(const name##2& v) noexcept {  \
    return v / length(v);                                               \
}                                                                       \
constexpr inline static name##3 normalize(const name##3& v) noexcept {  \
    return v / length(v);                                               \
}                                                                       \
constexpr inline static name##4 normalize(const name##4& v) noexcept {  \
    return v / length(v);                                               \
}

#define _prvl_MIX(ret, type, interpolator)                                                  \
constexpr inline static ret mix(const type a, const type b, const interpolator t) noexcept {\
    return a + t * (b - a);                                                                 \
}

#define _prvl_VEC_FUNC1(name, func, a)                              \
constexpr inline static name##2 func(const name##2& a) noexcept {   \
    return name##2(func(a.x), func(a.y));                           \
}                                                                   \
constexpr inline static name##3 func(const name##3& a) noexcept {   \
    return name##3(func(a.x), func(a.y), func(a.z));                \
}                                                                   \
constexpr inline static name##4 func(const name##4& a) noexcept {   \
    return name##4(func(a.x), func(a.y), func(a.z), func(a.w));     \
}

#define _prvl_VEC_FUNC2(name, func, a, b)                                           \
constexpr inline static name##2 func(const name##2& a, const name##2& b) noexcept { \
    return name##2(func(a.x, b.x), func(a.y, b.y));                                 \
}                                                                                   \
constexpr inline static name##3 func(const name##3& a, const name##3& b) noexcept { \
    return name##3(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z));                 \
}                                                                                   \
constexpr inline static name##4 func(const name##4& a, const name##4& b) noexcept { \
    return name##4(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z), func(a.w, b.w)); \
}

#define _prvl_VEC_FUNC3(name, func, a, b, c)                                                            \
constexpr inline static name##2 func(const name##2& a, const name##2& b, const name##2& c) noexcept {   \
    return name##2(func(a.x, b.x, c.x), func(a.y, b.y, c.y));                                           \
}                                                                                                       \
constexpr inline static name##3 func(const name##3& a, const name##3& b, const name##3& c) noexcept {   \
    return name##3(func(a.x, b.x, c.x), func(a.y, b.y, c.y), func(a.z, b.z, c.z));                      \
}                                                                                                       \
constexpr inline static name##4 func(const name##4& a, const name##4& b, const name##4& c) noexcept {   \
    return name##4(func(a.x, b.x, c.x), func(a.y, b.y, c.y), func(a.z, b.z, c.z), func(a.w, b.w, c.w)); \
}

#define _prvl_VEC_MIX(type, name)   \
_prvl_MIX(name##2, name##2&, type)  \
_prvl_MIX(name##3, name##3&, type)  \
_prvl_MIX(name##4, name##4&, type)

#define _prvl_VEC_ALL_OP(type, name)        \
_prvl_VEC_BINARY_OP(name, +)                \
_prvl_VEC_SCALAR_OP(type, name, +)          \
_prvl_VEC_BINARY_OP(name, -)                \
_prvl_VEC_SCALAR_OP(type, name, -)          \
_prvl_VEC_UNARY_OP(name, -)                 \
_prvl_VEC_BINARY_OP(name, *)                \
_prvl_VEC_SCALAR_OP(type, name, *)          \
_prvl_VEC_BINARY_OP(name, /)                \
_prvl_VEC_SCALAR_OP(type, name, /)          \
_prvl_VEC_DOT(type, name)                   \
_prvl_VEC_CROSS(name)                       \
_prvl_VEC_LENGTH(type, name)                \
_prvl_VEC_NORMALIZE(name)                   \
_prvl_VEC_MIX(type, name)                   \
_prvl_VEC_FUNC2(name, min, a, b)            \
_prvl_VEC_FUNC2(name, max, a, b)            \
_prvl_VEC_FUNC3(name, clamp, v, min, max)   \
_prvl_VEC_FUNC3(name, mix, a, b, t)         \
_prvl_VEC_FUNC2(name, mod, a, b)            \
_prvl_VEC_FUNC2(name, pow, a, b)            \
_prvl_VEC_CMP(name, <)                      \
_prvl_VEC_CMP(name, >)                      \
_prvl_VEC_CMP(name, <=)                     \
_prvl_VEC_CMP(name, >=)                     \
_prvl_VEC_CMP(name, ==)                     \
_prvl_VEC_CMP(name, !=)

#define _prvl_VEC_SIG_OP(name)  \
_prvl_VEC_FUNC1(name, abs, v)   \
_prvl_VEC_FUNC1(name, sign, v)

#define _prvl_VEC_DEC_OP(name)  \
_prvl_VEC_FUNC1(name, floor, v) \
_prvl_VEC_FUNC1(name, ceil, v)  \
_prvl_VEC_FUNC1(name, fract, v) \
_prvl_VEC_FUNC1(name, round, v) \
_prvl_VEC_FUNC1(name, trunc, v) \
_prvl_VEC_FUNC1(name, exp, v)   \
_prvl_VEC_FUNC1(name, log, v)   \
_prvl_VEC_FUNC1(name, sqrt, v)


#define _prvl_SCALAR_OP1(type, name, a, def)                \
constexpr inline static type name(const type a) noexcept {  \
    return def;                                             \
}

#define _prvl_SCALAR_OP2(type, name, a, b, def)                         \
constexpr inline static type name(const type a, const type b) noexcept {\
    return def;                                                         \
}

#define _prvl_SCALAR_OP3(type, name, a, b, c, def)                                      \
constexpr inline static type name(const type a, const type b, const type c) noexcept {  \
    return def;                                                                         \
}

#define _prvl_FUNC_ALL(type)                                                    \
_prvl_SCALAR_OP2(type, min, a, b, std::min(a, b))                               \
_prvl_SCALAR_OP2(type, max, a, b, std::max(a, b))                               \
_prvl_SCALAR_OP3(type, clamp, x, min, max, x < min ? min : (x > max ? max : x)) \
_prvl_SCALAR_OP3(type, mix, a, b, t, a + t * (b - a))

_prvl_FUNC_ALL(int32_t)
_prvl_FUNC_ALL(uint32_t)
_prvl_FUNC_ALL(float)
_prvl_FUNC_ALL(double)

_prvl_SCALAR_OP2(int32_t, mod, a, b, a % b)
_prvl_SCALAR_OP2(uint32_t, mod, a, b, a % b)
_prvl_SCALAR_OP2(float, mod, a, b, fmod(a, b))
_prvl_SCALAR_OP2(double, mod, a, b, fmod(a, b))

constexpr inline static int32_t pow(const int32_t a, const int32_t b) noexcept {
    int32_t result = 1;
    int32_t ap = a;
    int32_t r = b;
    while (r > 0) {
        if (r & 1) {
            result *= ap;
        }
        ap *= ap;
        r >>= 1;
    }
    return result;
}

constexpr inline static uint32_t pow(const uint32_t a, const uint32_t b) noexcept {
    uint32_t result = 1u;
    uint32_t ap = a;
    uint32_t r = b;
    while (r > 0u) {
        if (r & 1u) {
            result *= ap;
        }
        ap *= ap;
        r >>= 1u;
    }
    return result;
}

_prvl_SCALAR_OP1(int32_t, sign, x, x < 0 ? -1 : 1)
_prvl_SCALAR_OP1(float, sign, x, x < 0.0f ? -1.0f : 1.0f)
_prvl_SCALAR_OP1(double, sign, x, x < 0.0 ? -1.0 : 1.0)

_prvl_SCALAR_OP1(float, fract, x, x - floor(x))
_prvl_SCALAR_OP1(double, fract, x, x - floor(x))


_prvl_VEC_ALL(bool,     bvec)
_prvl_VEC_ALL(int32_t,  ivec)
_prvl_VEC_ALL(uint32_t, uvec)
_prvl_VEC_ALL(float,    vec)
_prvl_VEC_ALL(double,   dvec)

_prvl_IMPL_ALL(bool,     bvec)
_prvl_IMPL_ALL(int32_t,  ivec)
_prvl_IMPL_ALL(uint32_t, uvec)
_prvl_IMPL_ALL(float,    vec)
_prvl_IMPL_ALL(double,   dvec)

_prvl_CAST_ALL(bvec, ivec, bool)
_prvl_CAST_ALL(bvec, uvec, bool)
_prvl_CAST_ALL(bvec, vec,  bool)
_prvl_CAST_ALL(bvec, dvec, bool)

_prvl_CAST_ALL(ivec, bvec, int32_t)
_prvl_CAST_ALL(ivec, uvec, int32_t)
_prvl_CAST_ALL(ivec, vec,  int32_t)
_prvl_CAST_ALL(ivec, dvec, int32_t)

_prvl_CAST_ALL(uvec, bvec, uint32_t)
_prvl_CAST_ALL(uvec, ivec, uint32_t)
_prvl_CAST_ALL(uvec, vec,  uint32_t)
_prvl_CAST_ALL(uvec, dvec, uint32_t)

_prvl_CAST_ALL(vec, bvec, float)
_prvl_CAST_ALL(vec, ivec, float)
_prvl_CAST_ALL(vec, uvec, float)
_prvl_CAST_ALL(vec, dvec, float)

_prvl_CAST_ALL(dvec, bvec, double)
_prvl_CAST_ALL(dvec, ivec, double)
_prvl_CAST_ALL(dvec, uvec, double)
_prvl_CAST_ALL(dvec, vec,  double)

_prvl_VEC_ALL_OP(int32_t,  ivec)
_prvl_VEC_ALL_OP(uint32_t, uvec)
_prvl_VEC_ALL_OP(float,    vec)
_prvl_VEC_ALL_OP(double,   dvec)

_prvl_VEC_SIG_OP(ivec)
_prvl_VEC_SIG_OP(vec)
_prvl_VEC_SIG_OP(dvec)

_prvl_VEC_DEC_OP(vec)
_prvl_VEC_DEC_OP(dvec)

_prvl_VEC_BINARY_OP(ivec, &)
_prvl_VEC_SCALAR_OP(int32_t, ivec, &)
_prvl_VEC_BINARY_OP(ivec, |)
_prvl_VEC_SCALAR_OP(int32_t, ivec, |)
_prvl_VEC_BINARY_OP(ivec, ^)
_prvl_VEC_SCALAR_OP(int32_t, ivec, ^)
_prvl_VEC_BINARY_OP(ivec, <<)
_prvl_VEC_SCALAR_OP(int32_t, ivec, <<)
_prvl_VEC_BINARY_OP(ivec, >>)
_prvl_VEC_SCALAR_OP(int32_t, ivec, >>)
_prvl_VEC_UNARY_OP(ivec, ~)
_prvl_VEC_BINARY_OP(uvec, &)
_prvl_VEC_SCALAR_OP(uint32_t, uvec, &)
_prvl_VEC_BINARY_OP(uvec, |)
_prvl_VEC_SCALAR_OP(uint32_t, uvec, |)
_prvl_VEC_BINARY_OP(uvec, ^)
_prvl_VEC_SCALAR_OP(uint32_t, uvec, ^)
_prvl_VEC_BINARY_OP(uvec, <<)
_prvl_VEC_SCALAR_OP(uint32_t, uvec, <<)
_prvl_VEC_BINARY_OP(uvec, >>)
_prvl_VEC_SCALAR_OP(uint32_t, uvec, >>)
_prvl_VEC_UNARY_OP(uvec, ~)

_prvl_VEC_UNARY_OP(bvec, !)

constexpr inline static bool any(const bvec2& v) noexcept {
    return v.x || v.y;
}

constexpr inline static bool any(const bvec3& v) noexcept {
    return v.x || v.y || v.z;
}

constexpr inline static bool any(const bvec4& v) noexcept {
    return v.x || v.y || v.z || v.w;
}

constexpr inline static bool all(const bvec2& v) noexcept {
    return v.x && v.y;
}

constexpr inline static bool all(const bvec3& v) noexcept {
    return v.x && v.y && v.z;
}

constexpr inline static bool all(const bvec4& v) noexcept {
    return v.x && v.y && v.z && v.w;
}

template<typename T>
struct is_matrix : std::false_type {};

#define _prvl_MAT2XN(type, name, vecname, h)                                            \
struct name##2x##h {                                                                    \
    static constexpr uint32_t COLS = 2;                                                 \
    static constexpr uint32_t ROWS = h;                                                 \
    vecname##h _cols[2];                                                                \
    constexpr name##2x##h() : _cols{} {}                                                \
    constexpr name##2x##h(const vecname##h& a, const vecname##h& b) noexcept;           \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>      \
    explicit constexpr name##2x##h(const Other& other) noexcept;                        \
    constexpr inline vecname##h& operator[](const uint32_t idx) noexcept {              \
        return _cols[idx];                                                              \
    }                                                                                   \
    constexpr inline const vecname##h& operator[](const uint32_t idx) const noexcept {  \
        return _cols[idx];                                                              \
    }                                                                                   \
    constexpr inline type* data() noexcept {                                            \
        return &_cols[0].x;                                                             \
    }                                                                                   \
    constexpr inline const type* data() const noexcept {                                \
        return &_cols[0].x;                                                             \
    }                                                                                   \
};                                                                                      \
template<> struct is_matrix<name##2x##h> : std::true_type {};

#define _prvl_MAT3XN(type, name, vecname, h)                                                        \
struct name##3x##h {                                                                                \
    static constexpr uint32_t COLS = 3;                                                             \
    static constexpr uint32_t ROWS = h;                                                             \
    vecname##h _cols[3];                                                                            \
    constexpr name##3x##h() : _cols{} {}                                                            \
    constexpr name##3x##h(const vecname##h& a, const vecname##h& b, const vecname##h& c) noexcept;  \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                  \
    explicit constexpr name##3x##h(const Other& other) noexcept;                                    \
    constexpr inline vecname##h& operator[](const uint32_t idx) noexcept {                          \
        return _cols[idx];                                                                          \
    }                                                                                               \
    constexpr inline const vecname##h& operator[](const uint32_t idx) const noexcept {              \
        return _cols[idx];                                                                          \
    }                                                                                               \
    constexpr inline type* data() noexcept {                                                        \
        return &_cols[0].x;                                                                         \
    }                                                                                               \
    constexpr inline const type* data() const noexcept {                                            \
        return &_cols[0].x;                                                                         \
    }                                                                                               \
};                                                                                                  \
template<> struct is_matrix<name##3x##h> : std::true_type {};

#define _prvl_MAT4XN(type, name, vecname, h)                                                                            \
struct name##4x##h {                                                                                                    \
    static constexpr uint32_t COLS = 4;                                                                                 \
    static constexpr uint32_t ROWS = h;                                                                                 \
    vecname##h _cols[4];                                                                                                \
    constexpr name##4x##h() : _cols{} {}                                                                                \
    constexpr name##4x##h(const vecname##h& a, const vecname##h& b, const vecname##h& c, const vecname##h& d) noexcept; \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                                      \
    explicit constexpr name##4x##h(const Other& other) noexcept;                                                        \
    constexpr inline vecname##h& operator[](const uint32_t idx) noexcept {                                              \
        return _cols[idx];                                                                                              \
    }                                                                                                                   \
    constexpr inline const vecname##h& operator[](const uint32_t idx) const noexcept {                                  \
        return _cols[idx];                                                                                              \
    }                                                                                                                   \
    constexpr inline type* data() noexcept {                                                                            \
        return &_cols[0].x;                                                                                             \
    }                                                                                                                   \
    constexpr inline const type* data() const noexcept {                                                                \
        return &_cols[0].x;                                                                                             \
    }                                                                                                                   \
};                                                                                                                      \
template<> struct is_matrix<name##4x##h> : std::true_type {};

#define _prvl_IMPL_MAT2XN(type, name, vecname, h)                                       \
constexpr name##2x##h::name##2x##h(const vecname##h& a, const vecname##h& b) noexcept { \
    this->_cols[0] = a;                                                                 \
    this->_cols[1] = b;                                                                 \
}                                                                                       \
template<typename Other, typename>                                                      \
constexpr name##2x##h::name##2x##h(const Other& other) noexcept {                       \
    constexpr uint32_t copyCols = (COLS < Other::COLS) ? COLS : Other::COLS;            \
    constexpr uint32_t copyRows = (ROWS < Other::ROWS) ? ROWS : Other::ROWS;            \
    for (uint32_t c = 0u; c < copyCols; ++c) {                                          \
        for (uint32_t r = 0u; r < copyRows; ++r) {                                      \
            _cols[c][r] = other[c][r];                                                  \
        }                                                                               \
    }                                                                                   \
    if constexpr (COLS > copyCols) {                                                    \
        for (uint32_t c = copyCols; c < COLS; ++c) {                                    \
            _cols[c] = vecname##h{};                                                    \
        }                                                                               \
    }                                                                                   \
    if constexpr (ROWS > copyRows) {                                                    \
        for (uint32_t c = 0u; c < copyCols; ++c) {                                      \
            for (uint32_t r = copyRows; r < ROWS; ++r) {                                \
                _cols[c][r] = type(0);                                                  \
            }                                                                           \
        }                                                                               \
    }                                                                                   \
    constexpr uint32_t padding = (copyCols < copyRows) ? copyCols : copyRows;           \
    if constexpr (COLS == ROWS && padding < COLS) {                                     \
        for(uint32_t i = padding; i < COLS; i++) {                                      \
            _cols[i][i] = type(1);                                                      \
        }                                                                               \
    }                                                                                   \
}

#define _prvl_IMPL_MAT3XN(type, name, vecname, h)                                                           \
constexpr name##3x##h::name##3x##h(const vecname##h& a, const vecname##h& b, const vecname##h& c) noexcept {\
    this->_cols[0] = a;                                                                                     \
    this->_cols[1] = b;                                                                                     \
    this->_cols[2] = c;                                                                                     \
}                                                                                                           \
template<typename Other, typename>                                                                          \
constexpr name##3x##h::name##3x##h(const Other& other) noexcept {                                           \
    constexpr uint32_t copyCols = (COLS < Other::COLS) ? COLS : Other::COLS;                                \
    constexpr uint32_t copyRows = (ROWS < Other::ROWS) ? ROWS : Other::ROWS;                                \
    for (uint32_t c = 0u; c < copyCols; ++c) {                                                              \
        for (uint32_t r = 0u; r < copyRows; ++r) {                                                          \
            _cols[c][r] = other[c][r];                                                                      \
        }                                                                                                   \
    }                                                                                                       \
    if constexpr (COLS > copyCols) {                                                                        \
        for (uint32_t c = copyCols; c < COLS; ++c) {                                                        \
            _cols[c] = vecname##h{};                                                                        \
        }                                                                                                   \
    }                                                                                                       \
    if constexpr (ROWS > copyRows) {                                                                        \
        for (uint32_t c = 0u; c < copyCols; ++c) {                                                          \
            for (uint32_t r = copyRows; r < ROWS; ++r) {                                                    \
                _cols[c][r] = type(0);                                                                      \
            }                                                                                               \
        }                                                                                                   \
    }                                                                                                       \
    constexpr uint32_t padding = (copyCols < copyRows) ? copyCols : copyRows;                               \
    if constexpr (COLS == ROWS && padding < COLS) {                                                         \
        for(uint32_t i = padding; i < COLS; i++) {                                                          \
            _cols[i][i] = type(1);                                                                          \
        }                                                                                                   \
    }                                                                                                       \
}

#define _prvl_IMPL_MAT4XN(type, name, vecname, h)                                                                                   \
constexpr name##4x##h::name##4x##h(const vecname##h& a, const vecname##h& b, const vecname##h& c, const vecname##h& d) noexcept {   \
    this->_cols[0] = a;                                                                                                             \
    this->_cols[1] = b;                                                                                                             \
    this->_cols[2] = c;                                                                                                             \
    this->_cols[3] = d;                                                                                                             \
}                                                                                                                                   \
template<typename Other, typename>                                                                                                  \
constexpr name##4x##h::name##4x##h(const Other& other) noexcept {                                                                   \
    constexpr uint32_t copyCols = (COLS < Other::COLS) ? COLS : Other::COLS;                                                        \
    constexpr uint32_t copyRows = (ROWS < Other::ROWS) ? ROWS : Other::ROWS;                                                        \
    for (uint32_t c = 0u; c < copyCols; ++c) {                                                                                      \
        for (uint32_t r = 0u; r < copyRows; ++r) {                                                                                  \
            _cols[c][r] = other[c][r];                                                                                              \
        }                                                                                                                           \
    }                                                                                                                               \
    if constexpr (COLS > copyCols) {                                                                                                \
        for (uint32_t c = copyCols; c < COLS; ++c) {                                                                                \
            _cols[c] = vecname##h{};                                                                                                \
        }                                                                                                                           \
    }                                                                                                                               \
    if constexpr (ROWS > copyRows) {                                                                                                \
        for (uint32_t c = 0u; c < copyCols; ++c) {                                                                                  \
            for (uint32_t r = copyRows; r < ROWS; ++r) {                                                                            \
                _cols[c][r] = type(0);                                                                                              \
            }                                                                                                                       \
        }                                                                                                                           \
    }                                                                                                                               \
    constexpr uint32_t padding = (copyCols < copyRows) ? copyCols : copyRows;                                                       \
    if constexpr (COLS == ROWS && padding < COLS) {                                                                                 \
        for(uint32_t i = padding; i < COLS; i++) {                                                                                  \
            _cols[i][i] = type(1);                                                                                                  \
        }                                                                                                                           \
    }                                                                                                                               \
}

#define _prvl_MAT_ALL(type, name, vecname)  \
struct name##2x2;                           \
struct name##2x3;                           \
struct name##2x4;                           \
struct name##3x2;                           \
struct name##3x3;                           \
struct name##4x2;                           \
struct name##4x3;                           \
struct name##4x4;                           \
_prvl_MAT2XN(type, name, vecname, 2)        \
_prvl_MAT2XN(type, name, vecname, 3)        \
_prvl_MAT2XN(type, name, vecname, 4)        \
_prvl_MAT3XN(type, name, vecname, 2)        \
_prvl_MAT3XN(type, name, vecname, 3)        \
_prvl_MAT3XN(type, name, vecname, 4)        \
_prvl_MAT4XN(type, name, vecname, 2)        \
_prvl_MAT4XN(type, name, vecname, 3)        \
_prvl_MAT4XN(type, name, vecname, 4)        \
_prvl_IMPL_MAT2XN(type, name, vecname, 2)   \
_prvl_IMPL_MAT2XN(type, name, vecname, 3)   \
_prvl_IMPL_MAT2XN(type, name, vecname, 4)   \
_prvl_IMPL_MAT3XN(type, name, vecname, 2)   \
_prvl_IMPL_MAT3XN(type, name, vecname, 3)   \
_prvl_IMPL_MAT3XN(type, name, vecname, 4)   \
_prvl_IMPL_MAT4XN(type, name, vecname, 2)   \
_prvl_IMPL_MAT4XN(type, name, vecname, 3)   \
_prvl_IMPL_MAT4XN(type, name, vecname, 4)   \
using name##2 = name##2x2;                  \
using name##3 = name##3x3;                  \
using name##4 = name##4x4;


#define _prvl_MAT2_OP(name, h)                                                                  \
constexpr inline name##2x##h operator+(const name##2x##h& a, const name##2x##h& b) noexcept {   \
    return name##2x##h(a[0] + b[0], a[1] + b[1]);                                               \
}                                                                                               \
constexpr inline name##2x##h operator-(const name##2x##h& a, const name##2x##h& b) noexcept {   \
    return name##2x##h(a[0] - b[0], a[1] - b[1]);                                               \
}                                                                                               \
constexpr inline name##2x##h& operator+=(name##2x##h& a, const name##2x##h& b) noexcept {       \
    a[0] += b[0];                                                                               \
    a[1] += b[1];                                                                               \
    return a;                                                                                   \
}                                                                                               \
constexpr inline name##2x##h& operator-=(name##2x##h& a, const name##2x##h& b) noexcept {       \
    a[0] -= b[0];                                                                               \
    a[1] -= b[1];                                                                               \
    return a;                                                                                   \
}

#define _prvl_MAT3_OP(name, h)                                                                  \
constexpr inline name##3x##h operator+(const name##3x##h& a, const name##3x##h& b) noexcept {   \
    return name##3x##h(a[0] + b[0], a[1] + b[1], a[2] + b[2]);                                  \
}                                                                                               \
constexpr inline name##3x##h operator-(const name##3x##h& a, const name##3x##h& b) noexcept {   \
    return name##3x##h(a[0] - b[0], a[1] - b[1], a[2] - b[2]);                                  \
}                                                                                               \
constexpr inline name##3x##h& operator+=(name##3x##h& a, const name##3x##h& b) noexcept {       \
    a[0] += b[0];                                                                               \
    a[1] += b[1];                                                                               \
    a[2] += b[2];                                                                               \
    return a;                                                                                   \
}                                                                                               \
constexpr inline name##3x##h& operator-=(name##3x##h& a, const name##3x##h& b) noexcept {       \
    a[0] -= b[0];                                                                               \
    a[1] -= b[1];                                                                               \
    a[2] -= b[2];                                                                               \
    return a;                                                                                   \
}

#define _prvl_MAT4_OP(name, h)                                                                  \
constexpr inline name##4x##h operator+(const name##4x##h& a, const name##4x##h& b) noexcept {   \
    return name##4x##h(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);                     \
}                                                                                               \
constexpr inline name##4x##h operator-(const name##4x##h& a, const name##4x##h& b) noexcept {   \
    return name##4x##h(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);                     \
}                                                                                               \
constexpr inline name##4x##h& operator+=(name##4x##h& a, const name##4x##h& b) noexcept {       \
    a[0] += b[0];                                                                               \
    a[1] += b[1];                                                                               \
    a[2] += b[2];                                                                               \
    a[3] += b[3];                                                                               \
    return a;                                                                                   \
}                                                                                               \
constexpr inline name##4x##h& operator-=(name##4x##h& a, const name##4x##h& b) noexcept {       \
    a[0] -= b[0];                                                                               \
    a[1] -= b[1];                                                                               \
    a[2] -= b[2];                                                                               \
    a[3] -= b[3];                                                                               \
    return a;                                                                                   \
}

#define _prvl_MAT_MUL(type, name, m, n, p)                                                          \
constexpr inline name##p##x##m operator*(const name##n##x##m& a, const name##p##x##n& b) noexcept { \
    name##p##x##m r;                                                                                \
    _matmul<type, m, n, p>(a.data(), b.data(), r.data());                                           \
    return r;                                                                                       \
}

#define _prvl_MAT_MOD_MUL(name, n)                                              \
constexpr inline name##n& operator*=(name##n& a, const name##n& b) noexcept {   \
    a = a * b;                                                                  \
    return a;                                                                   \
}

#define _prvl_MAT_VEC_MUL2(name, vecname, h)                                                        \
constexpr inline static vecname##h operator*(const name##2x##h& a, const vecname##2& b) noexcept {  \
    return a[0] * b.x + a[1] * b.y;                                                                 \
}                                                                                                   \
constexpr inline static vecname##h operator*(const vecname##2& a, const name##2x##h& b) noexcept {  \
    return b[0] * a.x + b[1] * a.y;                                                                 \
}

#define _prvl_MAT_VEC_MUL3(name, vecname, h)                                                        \
constexpr inline static vecname##h operator*(const name##3x##h& a, const vecname##3& b) noexcept {  \
    return a[0] * b.x + a[1] * b.y + a[2] * b.z;                                                    \
}                                                                                                   \
constexpr inline static vecname##h operator*(const vecname##3& a, const name##3x##h& b) noexcept {  \
    return b[0] * a.x + b[1] * a.y + b[2] * a.z;                                                    \
}

#define _prvl_MAT_VEC_MUL4(name, vecname, h)                                                        \
constexpr inline static vecname##h operator*(const name##4x##h& a, const vecname##4& b) noexcept {  \
    return a[0] * b.x + a[1] * b.y + a[2] * b.z + a[3] * b.w;                                       \
}                                                                                                   \
constexpr inline static vecname##h operator*(const vecname##4& a, const name##4x##h& b) noexcept {  \
    return b[0] * a.x + b[1] * a.y + b[2] * a.z + b[3] * a.w;                                       \
}

#define _prvl_MAT_SCALAR_MUL2(type, name, h)                                                \
constexpr inline static name##2x##h operator*(const name##2x##h& a, const type b) noexcept {\
    name##2x##h r;                                                                          \
    type* rd = r.data();                                                                    \
    const type* ad = a.data();                                                              \
    for(int i = 0; i < 2 * h; i++) {                                                        \
        rd[i] = ad[i] * b;                                                                  \
    }                                                                                       \
    return r;                                                                               \
}                                                                                           \
constexpr inline static name##2x##h operator*(const type a, const name##2x##h& b) noexcept {\
    name##2x##h r;                                                                          \
    type* rd = r.data();                                                                    \
    const type* bd = b.data();                                                              \
    for(int i = 0; i < 2 * h; i++) {                                                        \
        rd[i] = bd[i] * a;                                                                  \
    }                                                                                       \
    return r;                                                                               \
}

#define _prvl_MAT_SCALAR_MUL3(type, name, h)                                                \
constexpr inline static name##3x##h operator*(const name##3x##h& a, const type b) noexcept {\
    name##3x##h r;                                                                          \
    type* rd = r.data();                                                                    \
    const type* ad = a.data();                                                              \
    for(int i = 0; i < 3 * h; i++) {                                                        \
        rd[i] = ad[i] * b;                                                                  \
    }                                                                                       \
    return r;                                                                               \
}                                                                                           \
constexpr inline static name##3x##h operator*(const type a, const name##3x##h& b) noexcept {\
    name##3x##h r;                                                                          \
    type* rd = r.data();                                                                    \
    const type* bd = b.data();                                                              \
    for(int i = 0; i < 3 * h; i++) {                                                        \
        rd[i] = bd[i] * a;                                                                  \
    }                                                                                       \
    return r;                                                                               \
}

#define _prvl_MAT_SCALAR_MUL4(type, name, h)                                                \
constexpr inline static name##4x##h operator*(const name##4x##h& a, const type b) noexcept {\
    name##4x##h r;                                                                          \
    type* rd = r.data();                                                                    \
    const type* ad = a.data();                                                              \
    for(int i = 0; i < 4 * h; i++) {                                                        \
        rd[i] = ad[i] * b;                                                                  \
    }                                                                                       \
    return r;                                                                               \
}                                                                                           \
constexpr inline static name##4x##h operator*(const type a, const name##4x##h& b) noexcept {\
    name##4x##h r;                                                                          \
    type* rd = r.data();                                                                    \
    const type* bd = b.data();                                                              \
    for(int i = 0; i < 4 * h; i++) {                                                        \
        rd[i] = bd[i] * a;                                                                  \
    }                                                                                       \
    return r;                                                                               \
}

#define _prvl_MAT_TRANSPOSE(type, name, w, h)                                       \
constexpr inline static name##h##x##w transpose(const name##w##x##h& m) noexcept {  \
    name##h##x##w r;                                                                \
    _transpose<type, w, h>(m.data(), r.data());                                     \
    return r;                                                                       \
}

#define _prvl_MAT_DETERMINANT(type, name)                               \
constexpr inline static type determinant(const name##2& m) noexcept {   \
    const type* d = m.data();                                           \
    return d[0] * d[3] - d[1] * d[2];                                   \
}                                                                       \
constexpr inline type determinant(const name##3& m) noexcept {          \
    const type* d = m.data();                                           \
    return d[0] * (d[4] * d[8] - d[5] * d[7])                           \
         - d[1] * (d[3] * d[8] - d[5] * d[6])                           \
         + d[2] * (d[3] * d[7] - d[4] * d[6]);                          \
}                                                                       \
constexpr inline static type determinant(const name##4& m) noexcept {   \
    const type* d = m.data();                                           \
    const type A = d[10] * d[15] - d[11] * d[14];                       \
    const type B = d[6] * d[11] - d[7] * d[10];                         \
    const type C = d[7] * d[14] - d[6] * d[15];                         \
    const type D = d[4] * d[9] - d[5] * d[8];                           \
    const type E = d[8] * d[13] - d[9] * d[12];                         \
    const type F = d[5] * d[12] - d[4] * d[13];                         \
    return d[0] * (A * d[5] +  B * d[13] + C * d[9])                    \
         - d[1] * (A * d[4] +  B * d[12] + C * d[8])                    \
         + d[2] * (D * d[15] + E * d[7] +  F * d[11])                   \
         - d[3] * (D * d[14] + E * d[6] +  F * d[10]);                  \
}

#define _prvl_MAT_INVERSE(type, name, vecname, unit, epsilon)   \
constexpr inline name##2 inverse(const name##2& m) {            \
    type det = m[0].x * m[1].y - m[0].y * m[1].x;               \
    if(abs(det) < epsilon) {                                    \
        return diag(vecname##2(1));                             \
    }                                                           \
    det = unit / det;                                           \
    name##2 r;                                                  \
    r[0].x =  m[1].y * det;                                     \
    r[0].y = -m[1].x * det;                                     \
    r[1].x = -m[0].y * det;                                     \
    r[1].y =  m[0].x * det;                                     \
    return r;                                                   \
}                                                               \
constexpr inline name##3 inverse(const name##3& m) {            \
    const type A = m[1].y * m[2].z;                             \
    const type B = m[1].z * m[2].y;                             \
    const type C = m[1].x * m[2].z;                             \
    const type D = m[1].z * m[2].x;                             \
    const type E = m[1].x * m[2].y;                             \
    const type F = m[1].y * m[2].x;                             \
    type det = (m[0].x * (A - B)                                \
              - m[0].y * (C - D)                                \
              + m[0].z * (E - F));                              \
    if(abs(det) < epsilon) {                                    \
        return diag(vecname##3(1));                             \
    }                                                           \
    det = unit / det;                                           \
    name##3 r;                                                  \
    r[0].x = (A - B) * det;                                     \
    r[0].y = (m[0].z * m[2].y - m[0].y * m[2].z) * det;         \
    r[0].z = (m[0].y * m[1].z - m[0].z * m[1].y) * det;         \
    r[1].x = (D - C) * det;                                     \
    r[1].y = (m[0].x * m[2].z - m[0].z * m[2].x) * det;         \
    r[1].z = (m[0].z * m[1].x - m[0].x * m[1].z) * det;         \
    r[2].x = (E - F) * det;                                     \
    r[2].y = (m[0].y * m[2].x - m[0].x * m[2].y) * det;         \
    r[2].z = (m[0].x * m[1].y - m[0].y * m[1].x) * det;         \
    return r;                                                   \
}                                                               \
constexpr inline name##4 inverse(const name##4& m) {            \
    name##4 r;                                                  \
    const type* d = m.data();                                   \
    r[0].x = d[5]  * ( d[10] * d[15] - d[11] * d[14])           \
           + d[9]  * (-d[6]  * d[15] + d[7]  * d[14])           \
           + d[13] * ( d[6]  * d[11] - d[7]  * d[10]);          \
    r[1].x = d[4]  * (-d[10] * d[15] + d[11] * d[14])           \
           + d[8]  * ( d[6]  * d[15] - d[7]  * d[14])           \
           + d[12] * (-d[6]  * d[11] + d[7]  * d[10]);          \
    r[2].x = d[4]  * ( d[9]  * d[15] - d[11] * d[13])           \
           + d[8]  * (-d[5]  * d[15] + d[7]  * d[13])           \
           + d[12] * ( d[5]  * d[11] - d[7]  * d[9] );          \
    r[3].x = d[4]  * (-d[9]  * d[14] + d[10] * d[13])           \
           + d[8]  * ( d[5]  * d[14] - d[6]  * d[13])           \
           + d[12] * (-d[5]  * d[10] + d[6]  * d[9] );          \
    type det = d[0] * r[0].x + d[1] * r[1].x                    \
             + d[2] * r[2].x + d[3] * r[3].x;                   \
    if(abs(det) < epsilon) {                                    \
        return diag(vecname##4(1));                             \
    }                                                           \
    det = unit / det;                                           \
    r[0].y = d[1]  * (-d[10] * d[15] + d[11] * d[14])           \
           + d[9]  * ( d[2]  * d[15] - d[3]  * d[14])           \
           + d[13] * (-d[2]  * d[11] + d[3]  * d[10]);          \
    r[1].y = d[0]  * ( d[10] * d[15] - d[11] * d[14])           \
           + d[8]  * ( d[2]  * d[15] + d[3]  * d[14])           \
           + d[12] * ( d[2]  * d[11] - d[3]  * d[10]);          \
    r[2].y = d[0]  * (-d[9]  * d[15] + d[11] * d[13])           \
           + d[8]  * ( d[1]  * d[15] - d[3]  * d[13])           \
           + d[12] * (-d[1]  * d[11] + d[3]  * d[9] );          \
    r[3].y = d[0]  * ( d[9]  * d[14] - d[10] * d[13])           \
           + d[8]  * (-d[1]  * d[14] + d[2]  * d[13])           \
           + d[12] * ( d[1]  * d[10] - d[2]  * d[9] );          \
    r[0].z = d[1]  * ( d[6]  * d[15] - d[7]  * d[14])           \
           + d[5]  * (-d[2]  * d[15] + d[3]  * d[14])           \
           + d[13] * ( d[2]  * d[7]  - d[3]  * d[6] );          \
    r[1].z = d[0]  * (-d[6]  * d[15] + d[7]  * d[14])           \
           + d[4]  * ( d[2]  * d[15] - d[3]  * d[14])           \
           + d[12] * (-d[2]  * d[7]  + d[3]  * d[6] );          \
    r[2].z = d[0]  * ( d[5]  * d[15] - d[7]  * d[13])           \
           + d[4]  * (-d[1]  * d[15] + d[3]  * d[13])           \
           + d[12] * ( d[1]  * d[7]  - d[3]  * d[5] );          \
    r[3].z = d[0]  * (-d[5]  * d[14] + d[6]  * d[13])           \
           + d[4]  * ( d[1]  * d[14] - d[2]  * d[13])           \
           + d[12] * (-d[1]  * d[6]  + d[2]  * d[5] );          \
    r[0].w = d[1]  * (-d[6]  * d[11] + d[7]  * d[10])           \
           + d[5]  * ( d[2]  * d[11] - d[3]  * d[10])           \
           + d[9]  * (-d[2]  * d[7]  + d[3]  * d[6] );          \
    r[1].w = d[0]  * ( d[6]  * d[11] - d[7]  * d[10])           \
           + d[4]  * (-d[2]  * d[11] + d[3]  * d[10])           \
           + d[8]  * ( d[2]  * d[7]  - d[3]  * d[6] );          \
    r[2].w = d[0]  * (-d[5]  * d[11] + d[7]  * d[9] )           \
           + d[4]  * ( d[1]  * d[11] - d[3]  * d[9] )           \
           + d[8]  * (-d[1]  * d[7]  + d[3]  * d[5] );          \
    r[3].w = d[0]  * ( d[5]  * d[10] - d[6]  * d[9] )           \
           + d[4]  * (-d[1]  * d[10] + d[2]  * d[9] )           \
           + d[8]  * ( d[1]  * d[6]  - d[2]  * d[5] );          \
    r[0].x *= det;                                              \
    r[0].y *= det;                                              \
    r[0].z *= det;                                              \
    r[0].w *= det;                                              \
    r[1].x *= det;                                              \
    r[1].y *= det;                                              \
    r[1].z *= det;                                              \
    r[1].w *= det;                                              \
    r[2].x *= det;                                              \
    r[2].y *= det;                                              \
    r[2].z *= det;                                              \
    r[2].w *= det;                                              \
    r[3].x *= det;                                              \
    r[3].y *= det;                                              \
    r[3].z *= det;                                              \
    r[3].w *= det;                                              \
    return r;                                                   \
}

#define _prvl_MAT_NORMALIZE(type, name)                                                 \
constexpr inline static name##2 normalize(const name##2& m) noexcept {                  \
    type det = determinant(m);                                                          \
    type scalar = static_cast<type>(1.0 / sqrt(static_cast<double>(det)));             \
    return m * scalar;                                                                  \
}                                                                                       \
constexpr inline static name##3 normalize(const name##3& m) noexcept {                  \
    type det = determinant(m);                                                          \
    type scalar = static_cast<type>(1.0 / pow(static_cast<double>(det), 1.0 / 3.0)); \
    return m * scalar;                                                                  \
}                                                                                       \
constexpr inline static name##4 normalize(const name##4& m) noexcept {                  \
    type det = determinant(m);                                                          \
    type scalar = static_cast<type>(1.0 / pow(static_cast<double>(det), 0.25));       \
    return m * scalar;                                                                  \
}

#define _prvl_MAT_ALL_OP(type, name, vecname)   \
_prvl_MAT2_OP(name, 2)                          \
_prvl_MAT2_OP(name, 3)                          \
_prvl_MAT2_OP(name, 4)                          \
_prvl_MAT3_OP(name, 2)                          \
_prvl_MAT3_OP(name, 3)                          \
_prvl_MAT3_OP(name, 4)                          \
_prvl_MAT4_OP(name, 2)                          \
_prvl_MAT4_OP(name, 3)                          \
_prvl_MAT4_OP(name, 4)                          \
_prvl_MAT_MUL(type, name, 2, 2, 2)              \
_prvl_MAT_MUL(type, name, 2, 2, 3)              \
_prvl_MAT_MUL(type, name, 2, 2, 4)              \
_prvl_MAT_MUL(type, name, 2, 3, 2)              \
_prvl_MAT_MUL(type, name, 2, 3, 3)              \
_prvl_MAT_MUL(type, name, 2, 3, 4)              \
_prvl_MAT_MUL(type, name, 2, 4, 2)              \
_prvl_MAT_MUL(type, name, 2, 4, 3)              \
_prvl_MAT_MUL(type, name, 2, 4, 4)              \
_prvl_MAT_MUL(type, name, 3, 2, 2)              \
_prvl_MAT_MUL(type, name, 3, 2, 3)              \
_prvl_MAT_MUL(type, name, 3, 2, 4)              \
_prvl_MAT_MUL(type, name, 3, 3, 2)              \
_prvl_MAT_MUL(type, name, 3, 3, 3)              \
_prvl_MAT_MUL(type, name, 3, 3, 4)              \
_prvl_MAT_MUL(type, name, 3, 4, 2)              \
_prvl_MAT_MUL(type, name, 3, 4, 3)              \
_prvl_MAT_MUL(type, name, 3, 4, 4)              \
_prvl_MAT_MUL(type, name, 4, 2, 2)              \
_prvl_MAT_MUL(type, name, 4, 2, 3)              \
_prvl_MAT_MUL(type, name, 4, 2, 4)              \
_prvl_MAT_MUL(type, name, 4, 3, 2)              \
_prvl_MAT_MUL(type, name, 4, 3, 3)              \
_prvl_MAT_MUL(type, name, 4, 3, 4)              \
_prvl_MAT_MUL(type, name, 4, 4, 2)              \
_prvl_MAT_MUL(type, name, 4, 4, 3)              \
_prvl_MAT_MUL(type, name, 4, 4, 4)              \
_prvl_MAT_MOD_MUL(name, 2)                      \
_prvl_MAT_MOD_MUL(name, 3)                      \
_prvl_MAT_MOD_MUL(name, 4)                      \
_prvl_MAT_VEC_MUL2(name, vecname, 2)            \
_prvl_MAT_VEC_MUL2(name, vecname, 3)            \
_prvl_MAT_VEC_MUL2(name, vecname, 4)            \
_prvl_MAT_VEC_MUL3(name, vecname, 2)            \
_prvl_MAT_VEC_MUL3(name, vecname, 3)            \
_prvl_MAT_VEC_MUL3(name, vecname, 4)            \
_prvl_MAT_VEC_MUL4(name, vecname, 2)            \
_prvl_MAT_VEC_MUL4(name, vecname, 3)            \
_prvl_MAT_VEC_MUL4(name, vecname, 4)            \
_prvl_MAT_SCALAR_MUL2(type, name, 2)            \
_prvl_MAT_SCALAR_MUL2(type, name, 3)            \
_prvl_MAT_SCALAR_MUL2(type, name, 4)            \
_prvl_MAT_SCALAR_MUL3(type, name, 2)            \
_prvl_MAT_SCALAR_MUL3(type, name, 3)            \
_prvl_MAT_SCALAR_MUL3(type, name, 4)            \
_prvl_MAT_SCALAR_MUL4(type, name, 2)            \
_prvl_MAT_SCALAR_MUL4(type, name, 3)            \
_prvl_MAT_SCALAR_MUL4(type, name, 4)            \
_prvl_MAT_TRANSPOSE(type, name, 2, 2)           \
_prvl_MAT_TRANSPOSE(type, name, 2, 3)           \
_prvl_MAT_TRANSPOSE(type, name, 2, 4)           \
_prvl_MAT_TRANSPOSE(type, name, 3, 2)           \
_prvl_MAT_TRANSPOSE(type, name, 3, 3)           \
_prvl_MAT_TRANSPOSE(type, name, 3, 4)           \
_prvl_MAT_TRANSPOSE(type, name, 4, 2)           \
_prvl_MAT_TRANSPOSE(type, name, 4, 3)           \
_prvl_MAT_TRANSPOSE(type, name, 4, 4)           \
_prvl_MAT_DETERMINANT(type, name)               \
_prvl_MAT_NORMALIZE(type, name)

#define _prvl_MAT_DIAG(name, vecname)                               \
constexpr inline static name##2 diag(const vecname##2& v) noexcept {\
    name##2 r;                                                      \
    r[0].x = v.x;                                                   \
    r[1].y = v.y;                                                   \
    return r;                                                       \
}                                                                   \
constexpr inline static name##3 diag(const vecname##3& v) noexcept {\
    name##3 r;                                                      \
    r[0].x = v.x;                                                   \
    r[1].y = v.y;                                                   \
    r[2].z = v.z;                                                   \
    return r;                                                       \
}                                                                   \
constexpr inline static name##4 diag(const vecname##4& v) noexcept {\
    name##4 r;                                                      \
    r[0].x = v.x;                                                   \
    r[1].y = v.y;                                                   \
    r[2].z = v.z;                                                   \
    r[3].w = v.w;                                                   \
    return r;                                                       \
}
#define _prvl_MAT_TOEPLITZ(name, vecname)                               \
constexpr inline static name##2 toeplitz(const vecname##2& v) noexcept {\
    name##2 r;                                                          \
    r[0] = v;                                                           \
    r[1].y = v.x;                                                       \
    return r;                                                           \
}                                                                       \
constexpr inline static name##3 toeplitz(const vecname##3& v) noexcept {\
    name##3 r;                                                          \
    r[0] = v;                                                           \
    r[1].y = v.x;                                                       \
    r[1].z = v.y;                                                       \
    r[2].z = v.x;                                                       \
    return r;                                                           \
}                                                                       \
constexpr inline static name##4 toeplitz(const vecname##4& v) noexcept {\
    name##4 r;                                                          \
    r[0] = v;                                                           \
    r[1].y = v.x;                                                       \
    r[1].z = v.y;                                                       \
    r[1].w = v.z;                                                       \
    r[2].z = v.x;                                                       \
    r[2].w = v.y;                                                       \
    r[3].w = v.x;                                                       \
    return r;                                                           \
}

#define _prvl_MAT_ALL_CREATE(type, name, vecname)   \
_prvl_MAT_DIAG(name, vecname)                       \
_prvl_MAT_TOEPLITZ(name, vecname)


_prvl_MAT_ALL(float, mat, vec)
_prvl_MAT_ALL(double, dmat, dvec)

// These matrix types doesn't exist in glsl, but I added them just because I could, and to show the power of macros
_prvl_MAT_ALL(bool, bmat, bvec)
_prvl_MAT_ALL(int32_t, imat, ivec)
_prvl_MAT_ALL(uint32_t, umat, uvec)

template<typename T, int M, int N, int P>
constexpr inline static void _matmul(const T* a, const T* b, T* out) noexcept {
    // a: M rows, N cols
    // b: N rows, P cols
    // out: M rows, P cols
    for (int col = 0; col < P; col++) {
        for (int row = 0; row < M; row++) {
            T sum{};
            for (int k = 0; k < N; k++) {
                sum += a[row + k * M] * b[k + col * N];
            }
            out[row + col * M] = sum;
        }
    }
}

template<typename T, int M, int N>
constexpr inline static void _transpose(const T* m, T* out) noexcept {
    // m: M rows, N cols
    // out: N rows, M cols
    for(int i = 0; i < M; i++) {
        for(int j = 0; j < N; j++) {
            out[i * N + j] = m[i + j * M];
        }
    }
}

_prvl_MAT_ALL_OP(float, mat, vec)
_prvl_MAT_ALL_OP(double, dmat, dvec)
_prvl_MAT_ALL_OP(int32_t, imat, ivec)
_prvl_MAT_ALL_OP(uint32_t, umat, uvec)

_prvl_MAT_ALL_CREATE(float, mat, vec)
_prvl_MAT_ALL_CREATE(double, dmat, dvec)
_prvl_MAT_ALL_CREATE(bool, bmat, bvec)
_prvl_MAT_ALL_CREATE(int32_t, imat, ivec)
_prvl_MAT_ALL_CREATE(uint32_t, umat, uvec)

_prvl_MAT_INVERSE(float, mat, vec, 1.0f, 0.0001f);
_prvl_MAT_INVERSE(double, dmat, dvec, 1.0, 0.00001);

constexpr inline static mat3 rotateX(float ang) {
    float sin = sinf(ang);
    float cos = cosf(ang);
    mat3 r;
    r[0][0] = 1.0f;
    r[1][1] = cos;
    r[1][2] = sin;
    r[2][1] = -sin;
    r[2][2] = cos;
    return r;
}

constexpr inline static mat3 rotateY(float ang) {
    float sin = sinf(ang);
    float cos = cosf(ang);
    mat3 r;
    r[1][1] = 1.0f;
    r[0][0] = cos;
    r[0][2] = sin;
    r[2][0] = -sin;
    r[2][2] = cos;
    return r;
}

constexpr inline static mat3 rotateZ(float ang) {
    float sin = sinf(ang);
    float cos = cosf(ang);
    mat3 r;
    r[2][2] = 1.0f;
    r[0][0] = cos;
    r[0][1] = sin;
    r[1][0] = -sin;
    r[1][1] = cos;
    return r;
}

constexpr inline static mat3 lookat(const vec3& from, const vec3& to) {
    vec3 f = normalize(from - to);
    vec3 r = normalize(vec3(-f.z, 0.0f, f.x));
    vec3 u = vec3(-r.z * f.y, r.z * f.x - r.x * f.z, r.x * f.y);
    return transpose(mat3(r, u, f));
}


constexpr inline static mat4 perspectiveProjection(const float fov, const float aspect, const float nearZ, const float farZ) {
    mat4 r;
    const float yScale = 1.0f / tanf(fov * 0.5f);
    const float xScale = yScale / aspect;
    const float mul = 1.0f / (farZ - nearZ);
    r[0][0] = xScale;
    r[1][1] = yScale;
    r[2][2] = -(farZ + nearZ) * mul;
    r[2][3] = -1.0f;
    r[3][2] = 2.0f * farZ * nearZ * mul;
    return r;
}

constexpr inline static mat4 reverseZPerspectiveProjection(const float fov, const float aspect, const float nearZ) {
    mat4 r;
    const float yScale = 1.0f / tanf(fov * 0.5f);
    const float xScale = yScale / aspect;
    r.data()[0] = xScale;
    r.data()[5] = yScale;
    r.data()[11] = -1.0f;
    r.data()[14] = nearZ;
    return r;
}

constexpr inline static void setPlane(const float a, const float b, const float c, const float d, const uint32_t idx, float* planes, bool* planeDirs) {
    float mul = 1.0f / sqrtf(a * a + b * b + c * c);
    planes[idx] = a * mul;
    planeDirs[idx] = a >= 0.0f;
    planes[idx + 1] = b * mul;
    planeDirs[idx + 1] = b >= 0.0f;
    planes[idx + 2] = c * mul;
    planeDirs[idx + 2] = c >= 0.0f;
    planes[idx + 3] = d * mul;
}

constexpr inline static void getViewFrustumPlanes(const mat4& proj, const mat4& cam, float* planes, bool* planeDirs) {
    mat4 r = proj * cam;
    const float* m = r.data();
    setPlane(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12], 0u, planes, planeDirs);
    setPlane(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12], 4u, planes, planeDirs);
    setPlane(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13], 8u, planes, planeDirs);
    setPlane(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13], 12u, planes, planeDirs);
    setPlane(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14], 16u, planes, planeDirs);
    //setPlane(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14], 20u, planes, planeDirs);
}

constexpr inline static bool isPointVisible(const float* planes, const vec3& p) {
    for (int i = 0; i < 5; i++) {
        int idx = i << 2;
        if (planes[idx] * p.x + planes[idx + 1] * p.y + planes[idx + 2] * p.z + planes[idx + 3] < 0.0f) {
            return false;
        }
    }
    return true;
}

constexpr inline static bool isAABBVisible(const float* planes, const bool* planeDirs, const vec3& min, const vec3& max) {
    for (int i = 0; i < 5; i++) {
        int idx = i << 2;
        float x = planeDirs[idx] ? max.x : min.x;
        float y = planeDirs[idx + 1] ? max.y : min.y;
        float z = planeDirs[idx + 2] ? max.z : min.z;
        if (planes[idx] * x + planes[idx + 1] * y + planes[idx + 2] * z + planes[idx + 3] < 0.0f) {
            return false;
        }
    }
    return true;
}

constexpr inline static void isMultiAABBVisible(const float* planes, const bool* planeDirs, const float* minX, const float* minY, const float* minZ, const float* maxX, const float* maxY, const float* maxZ, bool* out, const int count) {
    for (int i = 0; i < count; i++) {
        out[i] = true;
    }
    for (int i = 0; i < 5; i++) {
        int idx = i << 2;
        float a = planes[idx];
        float b = planes[idx + 1];
        float c = planes[idx + 2];
        float d = planes[idx + 3];
        const float* x = planeDirs[idx] ? maxX : minX;
        const float* y = planeDirs[idx + 1] ? maxY : minY;
        const float* z = planeDirs[idx + 2] ? maxZ : minZ;
        for (int j = 0; j < count; j++) {
            out[j] &= a * x[j] + b * y[j] + c * z[j] + d >= 0.0f;
        }
    }
}

#endif
