#ifndef PRVL_H_INCLUDED
#define PRVL_H_INCLUDED

#pragma once

#include <cstdint>
#include <cstdlib>
#include <cmath>

namespace prvl {
}

#define _prvl_VEC2(type, name)                                              \
struct name##2 {                                                            \
    union {                                                                 \
        struct {                                                            \
            type x;                                                         \
            type y;                                                         \
        };                                                                  \
        struct {                                                            \
            type r;                                                         \
            type g;                                                         \
        };                                                                  \
        struct {                                                            \
            type s;                                                         \
            type t;                                                         \
        };                                                                  \
    };                                                                      \
    constexpr type& operator[](const uint32_t idx) noexcept {               \
        return (&x)[idx];                                                   \
    }                                                                       \
    constexpr const type& operator[](const uint32_t idx) const noexcept {   \
        return (&x)[idx];                                                   \
    }                                                                       \
};                                                                          \
namespace prvl {                                                            \
    constexpr struct name##2 name##2() noexcept;                            \
    constexpr struct name##2 name##2(type x, type y) noexcept;              \
    constexpr struct name##2 name##2(const struct name##3& v) noexcept;     \
    constexpr struct name##2 name##2(const struct name##4& v) noexcept;     \
    constexpr struct name##2 name##2(type v) noexcept;                      \
}

#define _prvl_VEC3(type, name)                                                      \
struct name##3 {                                                                    \
    union {                                                                         \
        struct {                                                                    \
            type x;                                                                 \
            type y;                                                                 \
            type z;                                                                 \
        };                                                                          \
        struct {                                                                    \
            type r;                                                                 \
            type g;                                                                 \
            type b;                                                                 \
        };                                                                          \
        struct {                                                                    \
            type s;                                                                 \
            type t;                                                                 \
            type p;                                                                 \
        };                                                                          \
    };                                                                              \
    constexpr type& operator[](const uint32_t idx) noexcept {                       \
        return (&x)[idx];                                                           \
    }                                                                               \
    constexpr const type& operator[](const uint32_t idx) const noexcept {           \
        return (&x)[idx];                                                           \
    }                                                                               \
};                                                                                  \
namespace prvl {                                                                    \
    constexpr struct name##3 name##3() noexcept;                                    \
    constexpr struct name##3 name##3(type x, type y, type z) noexcept;              \
    constexpr struct name##3 name##3(const struct name##2& xy, type z) noexcept;    \
    constexpr struct name##3 name##3(type x, const struct name##2& yz) noexcept;    \
    constexpr struct name##3 name##3(const struct name##4& v) noexcept;             \
    constexpr struct name##3 name##3(type v) noexcept;                              \
}

#define _prvl_VEC4(type, name)                                                                      \
struct name##4 {                                                                                    \
    union {                                                                                         \
        struct {                                                                                    \
            type x;                                                                                 \
            type y;                                                                                 \
            type z;                                                                                 \
            type w;                                                                                 \
        };                                                                                          \
        struct {                                                                                    \
            type r;                                                                                 \
            type g;                                                                                 \
            type b;                                                                                 \
            type a;                                                                                 \
        };                                                                                          \
        struct {                                                                                    \
            type s;                                                                                 \
            type t;                                                                                 \
            type p;                                                                                 \
            type q;                                                                                 \
        };                                                                                          \
    };                                                                                              \
    constexpr type& operator[](const uint32_t idx) noexcept {                                       \
        return (&x)[idx];                                                                           \
    }                                                                                               \
    constexpr const type& operator[](const uint32_t idx) const noexcept {                           \
        return (&x)[idx];                                                                           \
    }                                                                                               \
};                                                                                                  \
namespace prvl {                                                                                    \
    constexpr struct name##4 name##4() noexcept;                                                    \
    constexpr struct name##4 name##4(type x, type y, type z, type w) noexcept;                      \
    constexpr struct name##4 name##4(const struct name##2& xy, type z, type w) noexcept;            \
    constexpr struct name##4 name##4(type x, const struct name##2& yz, type w) noexcept;            \
    constexpr struct name##4 name##4(type x, type y, const struct name##2& zw) noexcept;            \
    constexpr struct name##4 name##4(const struct name##2& xy, const struct name##2& zw) noexcept;  \
    constexpr struct name##4 name##4(const struct name##3& xyz, type w) noexcept;                   \
    constexpr struct name##4 name##4(type x, const struct name##3& yzw) noexcept;                   \
    constexpr struct name##4 name##4(type v) noexcept;                                              \
}


#define _prvl_VEC_ALL(type, name)   \
struct name##2;                     \
struct name##3;                     \
struct name##4;                     \
_prvl_VEC2(type, name)              \
_prvl_VEC3(type, name)              \
_prvl_VEC4(type, name)

#define _prvl_IMPL_VEC2(type, name)                                         \
constexpr struct name##2 prvl::name##2() noexcept {                         \
    return (struct name##2){0, 0};                                          \
}                                                                           \
constexpr struct name##2 prvl::name##2(type x, type y) noexcept {           \
    return (struct name##2){x, y};                                          \
}                                                                           \
constexpr struct name##2 prvl::name##2(const struct name##3& v) noexcept {  \
    return (struct name##2){v.x, v.y};                                      \
}                                                                           \
constexpr struct name##2 prvl::name##2(const struct name##4& v) noexcept {  \
    return (struct name##2){v.x, v.y};                                      \
}                                                                           \
constexpr struct name##2 prvl::name##2(type v) noexcept {                   \
    return (struct name##2){v, v};                                          \
}

#define _prvl_IMPL_VEC3(type, name)                                                 \
constexpr struct name##3 prvl::name##3() noexcept {                                 \
    return (struct name##3){0, 0, 0};                                               \
}                                                                                   \
constexpr struct name##3 prvl::name##3(type x, type y, type z) noexcept {           \
    return (struct name##3){x, y, z};                                               \
}                                                                                   \
constexpr struct name##3 prvl::name##3(const struct name##2& xy, type z) noexcept { \
    return (struct name##3){xy.x, xy.y, z};                                         \
}                                                                                   \
constexpr struct name##3 prvl::name##3(type x, const struct name##2& yz) noexcept { \
    return (struct name##3){x, yz.x, yz.y};                                         \
}                                                                                   \
constexpr struct name##3 prvl::name##3(const struct name##4& v) noexcept {          \
    return (struct name##3){v.x, v.y, v.z};                                         \
}                                                                                   \
constexpr struct name##3 prvl::name##3(type v) noexcept {                           \
    return (struct name##3){v, v, v};                                               \
}

#define _prvl_IMPL_VEC4(type, name)                                                                     \
constexpr struct name##4 prvl::name##4() noexcept {                                                     \
    return (struct name##4){0, 0, 0, 0};                                                                \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(type x, type y, type z, type w) noexcept {                       \
    return (struct name##4){x, y, z, w};                                                                \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(const struct name##2& xy, type z, type w) noexcept {             \
    return (struct name##4){xy.x, xy.y, z, w};                                                          \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(type x, const struct name##2& yz, type w) noexcept {             \
    return (struct name##4){x, yz.x, yz.y, w};                                                          \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(type x, type y, const struct name##2& zw) noexcept {             \
    return (struct name##4){x, y, zw.x, zw.y};                                                          \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(const struct name##2& xy, const struct name##2& zw) noexcept {   \
    return (struct name##4){xy.x, xy.y, zw.x, zw.y};                                                    \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(const struct name##3& xyz, type w) noexcept {                    \
    return (struct name##4){xyz.x, xyz.y, xyz.z, w};                                                    \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(type x, const struct name##3& yzw) noexcept {                    \
    return (struct name##4){x, yzw.x, yzw.y, yzw.z};                                                    \
}                                                                                                       \
constexpr struct name##4 prvl::name##4(type v) noexcept {                                               \
    return (struct name##4){v, v, v, v};                                                                \
}

#define _prvl_IMPL_ALL(type, name)  \
_prvl_IMPL_VEC2(type, name)         \
_prvl_IMPL_VEC3(type, name)         \
_prvl_IMPL_VEC4(type, name)

#define _prvl_CAST2(nameA, nameB, typeA)                                    \
namespace prvl {                                                            \
    constexpr struct nameA##2 nameA##2(const struct nameB##2& v) noexcept { \
        return nameA##2(static_cast<typeA>(v.x), static_cast<typeA>(v.y));  \
    }                                                                       \
}

#define _prvl_CAST3(nameA, nameB, typeA)                                                            \
namespace prvl {                                                                                    \
    constexpr struct nameA##3 nameA##3(const struct nameB##3& v) noexcept {                         \
        return nameA##3(static_cast<typeA>(v.x), static_cast<typeA>(v.y), static_cast<typeA>(v.z)); \
    }                                                                                               \
}

#define _prvl_CAST4(nameA, nameB, typeA)                                                                                        \
namespace prvl {                                                                                                                \
    constexpr struct nameA##4 nameA##4(const struct nameB##4& v) noexcept {                                                     \
        return nameA##4(static_cast<typeA>(v.x), static_cast<typeA>(v.y), static_cast<typeA>(v.z), static_cast<typeA>(v.w));    \
    }                                                                                                                           \
}

#define _prvl_CAST_ALL(to, from, type)  \
_prvl_CAST2(to, from, type)             \
_prvl_CAST3(to, from, type)             \
_prvl_CAST4(to, from, type)

#define _prvl_VEC_BINARY_OP(name, op)                                           \
constexpr name##2 operator op(const name##2& a, const name##2& b) noexcept {    \
    return prvl::name##2(a.x op b.x, a.y op b.y);                               \
}                                                                               \
constexpr name##2& operator op##=(name##2& a, const name##2& b) noexcept {      \
    a.x op##= b.x;                                                              \
    a.y op##= b.y;                                                              \
    return a;                                                                   \
}                                                                               \
constexpr name##3 operator op(const name##3& a, const name##3& b) noexcept {    \
    return prvl::name##3(a.x op b.x, a.y op b.y, a.z op b.z);                   \
}                                                                               \
constexpr name##3& operator op##=(name##3& a, const name##3& b) noexcept {      \
    a.x op##= b.x;                                                              \
    a.y op##= b.y;                                                              \
    a.z op##= b.z;                                                              \
    return a;                                                                   \
}                                                                               \
constexpr name##4 operator op(const name##4& a, const name##4& b) noexcept {    \
    return prvl::name##4(a.x op b.x, a.y op b.y, a.z op b.z, a.w op b.w);       \
}                                                                               \
constexpr name##4& operator op##=(name##4& a, const name##4& b) noexcept {      \
    a.x op##= b.x;                                                              \
    a.y op##= b.y;                                                              \
    a.z op##= b.z;                                                              \
    a.w op##= b.w;                                                              \
    return a;                                                                   \
}

#define _prvl_VEC_UNARY_OP(name, op)                        \
constexpr name##2 operator op(const name##2& v) noexcept {  \
    return prvl::name##2(op v.x, op v.y);                   \
}                                                           \
constexpr name##3 operator op(const name##3& v) noexcept {  \
    return prvl::name##3(op v.x, op v.y, op v.z);           \
}                                                           \
constexpr name##4 operator op(const name##4& v) noexcept {  \
    return prvl::name##4(op v.x, op v.y, op v.z, op v.w);   \
}

#define _prvl_VEC_SCALAR_OP(type, name, op)                                 \
constexpr name##2 operator op(const name##2& a, const type b) noexcept {    \
    return prvl::name##2(a.x op b, a.y op b);                               \
}                                                                           \
constexpr name##2& operator op##=(name##2& a, const type b) noexcept {      \
    a.x op##= b;                                                            \
    a.y op##= b;                                                            \
    return a;                                                               \
}                                                                           \
constexpr name##2 operator op(const type a, const name##2& b) noexcept {    \
    return prvl::name##2(a op b.x, a op b.y);                               \
}                                                                           \
constexpr name##3 operator op(const name##3& a, const type b) noexcept {    \
    return prvl::name##3(a.x op b, a.y op b, a.z op b);                     \
}                                                                           \
constexpr name##3& operator op##=(name##3& a, const type b) noexcept {      \
    a.x op##= b;                                                            \
    a.y op##= b;                                                            \
    a.z op##= b;                                                            \
    return a;                                                               \
}                                                                           \
constexpr name##3 operator op(const type a, const name##3& b) noexcept {    \
    return prvl::name##3(a op b.x, a op b.y, a op b.z);                     \
}                                                                           \
constexpr name##4 operator op(const name##4& a, const type b) noexcept {    \
    return prvl::name##4(a.x op b, a.y op b, a.z op b, a.w op b);           \
}                                                                           \
constexpr name##4& operator op##=(name##4& a, const type b) noexcept {      \
    a.x op##= b;                                                            \
    a.y op##= b;                                                            \
    a.z op##= b;                                                            \
    a.w op##= b;                                                            \
    return a;                                                               \
}                                                                           \
constexpr name##4 operator op(const type a, const name##4& b) noexcept {    \
    return prvl::name##4(a op b.x, a op b.y, a op b.z, a op b.w);           \
}                                                                           \

#define _prvl_VEC_CMP(name, cmp)                                            \
constexpr bvec2 operator cmp(const name##2& a, const name##2& b) noexcept { \
    return prvl::bvec2(a.x cmp b.x, a.y cmp b.y);                           \
}                                                                           \
constexpr bvec3 operator cmp(const name##3& a, const name##3& b) noexcept { \
    return prvl::bvec3(a.x cmp b.x, a.y cmp b.y, a.z cmp b.z);              \
}                                                                           \
constexpr bvec4 operator cmp(const name##4& a, const name##4& b) noexcept { \
    return prvl::bvec4(a.x cmp b.x, a.y cmp b.y, a.z cmp b.z, a.w cmp b.w); \
}

#define _prvl_VEC_DOT(type, name)                                   \
constexpr type dot(const name##2& a, const name##2& b) noexcept {   \
    return a.x * b.x + a.y * b.y;                                   \
}                                                                   \
constexpr type dot(const name##3& a, const name##3& b) noexcept {   \
    return a.x * b.x + a.y * b.y + a.z * b.z;                       \
}                                                                   \
constexpr type dot(const name##4& a, const name##4& b) noexcept {   \
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;           \
}

#define _prvl_VEC_CROSS(name)                                                                   \
constexpr name##3 cross(const name##3& a, const name##3& b) noexcept {                          \
    return prvl::name##3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);  \
}

#define _prvl_VEC_LENGTH(type, name)                                \
constexpr type length(const name##2& v) noexcept {                  \
    return sqrtf(v.x * v.x + v.y * v.y);                            \
}                                                                   \
constexpr type length(const name##3& v) noexcept {                  \
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);                \
}                                                                   \
constexpr type length(const name##4& v) noexcept {                  \
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);    \
}

#define _prvl_VEC_NORMALIZE(name)                           \
constexpr name##2 normalize(const name##2& v) noexcept {    \
    return v / length(v);                                   \
}                                                           \
constexpr name##3 normalize(const name##3& v) noexcept {    \
    return v / length(v);                                   \
}                                                           \
constexpr name##4 normalize(const name##4& v) noexcept {    \
    return v / length(v);                                   \
}

#define _prvl_MIX(ret, type, interpolator)                                      \
constexpr ret mix(const type a, const type b, const interpolator t) noexcept {  \
    return a + t * (b - a);                                                     \
}

#define _prvl_VEC_FUNC1(name, func, a)                                  \
constexpr name##2 func(const name##2& a) noexcept {                     \
    return prvl::name##2(func(a.x), func(a.y));                         \
}                                                                       \
constexpr name##3 func(const name##3& a) noexcept {                     \
    return prvl::name##3(func(a.x), func(a.y), func(a.z));              \
}                                                                       \
constexpr name##4 func(const name##4& a) noexcept {                     \
    return prvl::name##4(func(a.x), func(a.y), func(a.z), func(a.w));   \
}

#define _prvl_VEC_FUNC2(name, func, a, b)                                                   \
constexpr name##2 func(const name##2& a, const name##2& b) noexcept {                       \
    return prvl::name##2(func(a.x, b.x), func(a.y, b.y));                                   \
}                                                                                           \
constexpr name##3 func(const name##3& a, const name##3& b) noexcept {                       \
    return prvl::name##3(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z));                   \
}                                                                                           \
constexpr name##4 func(const name##4& a, const name##4& b) noexcept {                       \
    return prvl::name##4(func(a.x, b.x), func(a.y, b.y), func(a.z, b.z), func(a.w, b.w));   \
}

#define _prvl_VEC_FUNC3(name, func, a, b, c)                                                                    \
constexpr name##2 func(const name##2& a, const name##2& b, const name##2& c) noexcept {                         \
    return prvl::name##2(func(a.x, b.x, c.x), func(a.y, b.y, c.y));                                             \
}                                                                                                               \
constexpr name##3 func(const name##3& a, const name##3& b, const name##3& c) noexcept {                         \
    return prvl::name##3(func(a.x, b.x, c.x), func(a.y, b.y, c.y), func(a.z, b.z, c.z));                        \
}                                                                                                               \
constexpr name##4 func(const name##4& a, const name##4& b, const name##4& c) noexcept {                         \
    return prvl::name##4(func(a.x, b.x, c.x), func(a.y, b.y, c.y), func(a.z, b.z, c.z), func(a.w, b.w, c.w));   \
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


#define _prvl_SCALAR_OP1(type, name, a, def)    \
constexpr type name(type a) noexcept {          \
    return def;                                 \
}

#define _prvl_SCALAR_OP2(type, name, a, b, def) \
constexpr type name(type a, type b) noexcept {  \
    return def;                                 \
}

#define _prvl_SCALAR_OP3(type, name, a, b, c, def)      \
constexpr type name(type a, type b, type c) noexcept {  \
    return def;                                         \
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
_prvl_SCALAR_OP2(float, mod, a, b, fmodf(a, b))
_prvl_SCALAR_OP2(double, mod, a, b, fmod(a, b))

constexpr int32_t pow(int32_t a, int32_t b) noexcept {
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

constexpr uint32_t pow(const uint32_t a, const uint32_t b) noexcept {
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

constexpr bool any(const bvec2& v) noexcept {
    return v.x || v.y;
}

constexpr bool any(const bvec3& v) noexcept {
    return v.x || v.y || v.z;
}

constexpr bool any(const bvec4& v) noexcept {
    return v.x || v.y || v.z || v.w;
}

constexpr bool all(const bvec2& v) noexcept {
    return v.x && v.y;
}

constexpr bool all(const bvec3& v) noexcept {
    return v.x && v.y && v.z;
}

constexpr bool all(const bvec4& v) noexcept {
    return v.x && v.y && v.z && v.w;
}

struct quat {
    float x;
    float y;
    float z;
    float w;
};

struct dquat {
    double x;
    double y;
    double z;
    double w;
};

namespace prvl {
    constexpr struct quat quat() noexcept {
        return (struct quat){0.0f, 0.0f, 0.0f, 1.0f};
    }

    constexpr struct quat quat(float x, float y, float z, float w) noexcept {
        return (struct quat){x, y, z, w};
    }

    constexpr struct quat quat(const struct vec3& axis, float angle) noexcept {
        float half = angle * 0.5f;
        float s = sinf(half);
        return (struct quat){axis.x * s, axis.y * s, axis.z * s, cosf(half)};
    }

    constexpr struct dquat dquat() noexcept {
        return (struct dquat){0.0, 0.0, 0.0, 1.0};
    }

    constexpr struct dquat dquat(double x, double y, double z, double w) noexcept {
        return (struct dquat){x, y, z, w};
    }

    constexpr struct dquat dquat(const struct dvec3& axis, double angle) noexcept {
        double half = angle * 0.5;
        double s = sin(half);
        return (struct dquat){axis.x * s, axis.y * s, axis.z * s, cos(half)};
    }
}

constexpr quat operator+(const quat& a, const quat& b) {
    return quat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

constexpr quat& operator+=(quat& a, const quat& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

constexpr quat operator-(const quat& q) {
    return quat(-q.x, -q.y, -q.z, -q.w);
}

constexpr quat operator*(const quat& q, float s) {
    return quat(q.x * s, q.y * s, q.z * s, q.w * s);
}

constexpr quat& operator*=(quat& q, float s) {
    q.x *= s;
    q.y *= s;
    q.z *= s;
    q.w *= s;
    return q;
}

constexpr quat operator*(const quat& a, const quat& b) {
    return quat(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

constexpr quat& operator*=(quat& a, const quat& b) {
    float x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    float y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    float z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    float w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    a.x = x;
    a.y = y;
    a.z = z;
    a.w = w;
    return a;
}

constexpr quat operator/(const quat& q, float s) {
    return quat(q.x / s, q.y / s, q.z / s, q.w / s);
}

constexpr quat& operator/=(quat& q, float s) {
    q.x /= s;
    q.y /= s;
    q.z /= s;
    q.w /= s;
    return q;
}

constexpr float dot(const quat& a, const quat& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr quat normalize(const quat& q) {
    float inv = 1.0f / sqrtf(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    return quat(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
}

constexpr quat conjugate(const quat& q) {
    return quat(-q.x, -q.y, -q.z, q.w);
}

constexpr quat inverse(const quat& q) {
    return conjugate(q) / dot(q, q);
}

constexpr quat slerp(const quat& a, quat b, float t) {
    float d = dot(a, b);
    if(d < 0.0f) {
        b = -b;
        d = -d;
    }
    if(d > 0.999f) {
        return a * (1.0f - t) + b * t;
    }
    float ang = acosf(d);
    float s = sinf(ang);
    float wa = sinf((1.0f - t) * ang) / s;
    float wb = sinf(t * ang) / s;
    return a * wa + b * wb;
}

constexpr dquat operator+(const dquat& a, const dquat& b) {
    return dquat(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
}

constexpr dquat& operator+=(dquat& a, const dquat& b) {
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    a.w += b.w;
    return a;
}

constexpr dquat operator-(const dquat& q) {
    return dquat(-q.x, -q.y, -q.z, -q.w);
}

constexpr dquat operator*(const dquat& q, double s) {
    return dquat(q.x * s, q.y * s, q.z * s, q.w * s);
}

constexpr dquat& operator*=(dquat& q, double s) {
    q.x *= s;
    q.y *= s;
    q.z *= s;
    q.w *= s;
    return q;
}

constexpr dquat operator*(const dquat& a, const dquat& b) {
    return dquat(
        a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y,
        a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x,
        a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w,
        a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z
    );
}

constexpr dquat& operator*=(dquat& a, const dquat& b) {
    double x = a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y;
    double y = a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x;
    double z = a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w;
    double w = a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z;
    a.x = x;
    a.y = y;
    a.z = z;
    a.w = w;
    return a;
}

constexpr dquat operator/(const dquat& q, double s) {
    return dquat(q.x / s, q.y / s, q.z / s, q.w / s);
}

constexpr dquat& operator/=(dquat& q, double s) {
    q.x /= s;
    q.y /= s;
    q.z /= s;
    q.w /= s;
    return q;
}

constexpr double dot(const dquat& a, const dquat& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

constexpr dquat normalize(const dquat& q) {
    double inv = 1.0 / sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    return dquat(q.x * inv, q.y * inv, q.z * inv, q.w * inv);
}

constexpr dquat conjugate(const dquat& q) {
    return dquat(-q.x, -q.y, -q.z, q.w);
}

constexpr dquat inverse(const dquat& q) {
    return conjugate(q) / dot(q, q);
}

constexpr dquat slerp(const dquat& a, dquat b, double t) {
    double d = dot(a, b);
    if(d < 0.0) {
        b = -b;
        d = -d;
    }
    if(d > 0.9999) {
        return a * (1.0 - t) + b * t;
    }
    double ang = acos(d);
    double s = sin(ang);
    double wa = sin((1.0 - t) * ang) / s;
    double wb = sin(t * ang) / s;
    return a * wa + b * wb;
}

template<typename T>
struct is_matrix : std::false_type {};

#define _prvl_MAT2XN(type, name, vecname, h)                                                                    \
struct name##2x##h {                                                                                            \
    static constexpr uint32_t COLS = 2;                                                                         \
    static constexpr uint32_t ROWS = h;                                                                         \
    vecname##h _cols[2];                                                                                        \
    constexpr vecname##h& operator[](const uint32_t idx) noexcept {                                             \
        return _cols[idx];                                                                                      \
    }                                                                                                           \
    constexpr const vecname##h& operator[](const uint32_t idx) const noexcept {                                 \
        return _cols[idx];                                                                                      \
    }                                                                                                           \
    constexpr type* data() noexcept {                                                                           \
        return &_cols[0].x;                                                                                     \
    }                                                                                                           \
    constexpr const type* data() const noexcept {                                                               \
        return &_cols[0].x;                                                                                     \
    }                                                                                                           \
};                                                                                                              \
template<> struct is_matrix<name##2x##h> : std::true_type {};                                                   \
namespace prvl {                                                                                                \
    constexpr struct name##2x##h name##2x##h() noexcept;                                                        \
    constexpr struct name##2x##h name##2x##h(const struct vecname##h& a, const struct vecname##h& b) noexcept;  \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                              \
    constexpr struct name##2x##h name##2x##h(const Other& other) noexcept;                                      \
}

#define _prvl_MAT3XN(type, name, vecname, h)                                                                                                \
struct name##3x##h {                                                                                                                        \
    static constexpr uint32_t COLS = 3;                                                                                                     \
    static constexpr uint32_t ROWS = h;                                                                                                     \
    vecname##h _cols[3];                                                                                                                    \
    constexpr vecname##h& operator[](const uint32_t idx) noexcept {                                                                         \
        return _cols[idx];                                                                                                                  \
    }                                                                                                                                       \
    constexpr const vecname##h& operator[](const uint32_t idx) const noexcept {                                                             \
        return _cols[idx];                                                                                                                  \
    }                                                                                                                                       \
    constexpr type* data() noexcept {                                                                                                       \
        return &_cols[0].x;                                                                                                                 \
    }                                                                                                                                       \
    constexpr const type* data() const noexcept {                                                                                           \
        return &_cols[0].x;                                                                                                                 \
    }                                                                                                                                       \
};                                                                                                                                          \
template<> struct is_matrix<name##3x##h> : std::true_type {};                                                                               \
namespace prvl {                                                                                                                            \
    constexpr struct name##3x##h name##3x##h() noexcept;                                                                                    \
    constexpr struct name##3x##h name##3x##h(const struct vecname##h& a, const struct vecname##h& b, const struct vecname##h& c) noexcept;  \
    constexpr struct name##3x##h name##3x##h(const struct quat& q) noexcept;                                                                \
    constexpr struct name##3x##h name##3x##h(const struct dquat& q) noexcept;                                                               \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                                                          \
    constexpr struct name##3x##h name##3x##h(const Other& other) noexcept;                                                                  \
}

#define _prvl_MAT4XN(type, name, vecname, h)                                                                                                                            \
struct name##4x##h {                                                                                                                                                    \
    static constexpr uint32_t COLS = 4;                                                                                                                                 \
    static constexpr uint32_t ROWS = h;                                                                                                                                 \
    vecname##h _cols[4];                                                                                                                                                \
    constexpr vecname##h& operator[](const uint32_t idx) noexcept {                                                                                                     \
        return _cols[idx];                                                                                                                                              \
    }                                                                                                                                                                   \
    constexpr const vecname##h& operator[](const uint32_t idx) const noexcept {                                                                                         \
        return _cols[idx];                                                                                                                                              \
    }                                                                                                                                                                   \
    constexpr type* data() noexcept {                                                                                                                                   \
        return &_cols[0].x;                                                                                                                                             \
    }                                                                                                                                                                   \
    constexpr const type* data() const noexcept {                                                                                                                       \
        return &_cols[0].x;                                                                                                                                             \
    }                                                                                                                                                                   \
};                                                                                                                                                                      \
template<> struct is_matrix<name##4x##h> : std::true_type {};                                                                                                           \
namespace prvl {                                                                                                                                                        \
    constexpr struct name##4x##h name##4x##h() noexcept;                                                                                                                \
    constexpr struct name##4x##h name##4x##h(const struct vecname##h& a, const struct vecname##h& b, const struct vecname##h& c, const struct vecname##h& d) noexcept;  \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                                                                                      \
    constexpr struct name##4x##h name##4x##h(const Other& other) noexcept;                                                                                              \
}

#define _prvl_IMPL_MAT2XN(type, name, vecname, h)                                                                   \
constexpr struct name##2x##h prvl::name##2x##h() noexcept {                                                         \
    return (struct name##2x##h){prvl::vecname##h(), prvl::vecname##h()};                                            \
}                                                                                                                   \
constexpr struct name##2x##h prvl::name##2x##h(const struct vecname##h& a, const struct vecname##h& b) noexcept {   \
    return (struct name##2x##h){a, b};                                                                              \
}                                                                                                                   \
template<typename Other, typename>                                                                                  \
constexpr struct name##2x##h prvl::name##2x##h(const Other& other) noexcept {                                       \
    constexpr uint32_t copyCols = (name##2x##h::COLS < Other::COLS) ? name##2x##h::COLS : Other::COLS;              \
    constexpr uint32_t copyRows = (name##2x##h::ROWS < Other::ROWS) ? name##2x##h::ROWS : Other::ROWS;              \
    struct name##2x##h res;                                                                                         \
    for (uint32_t c = 0u; c < copyCols; ++c) {                                                                      \
        for (uint32_t r = 0u; r < copyRows; ++r) {                                                                  \
            res._cols[c][r] = other[c][r];                                                                          \
        }                                                                                                           \
    }                                                                                                               \
    if constexpr (name##2x##h::COLS > copyCols) {                                                                   \
        for (uint32_t c = copyCols; c < name##2x##h::COLS; ++c) {                                                   \
            res._cols[c] = prvl::vecname##h();                                                                      \
        }                                                                                                           \
    }                                                                                                               \
    if constexpr (name##2x##h::ROWS > copyRows) {                                                                   \
        for (uint32_t c = 0u; c < copyCols; ++c) {                                                                  \
            for (uint32_t r = copyRows; r < name##2x##h::ROWS; ++r) {                                               \
                res._cols[c][r] = type(0);                                                                          \
            }                                                                                                       \
        }                                                                                                           \
    }                                                                                                               \
    constexpr uint32_t padding = (copyCols < copyRows) ? copyCols : copyRows;                                       \
    if constexpr (name##2x##h::COLS == name##2x##h::ROWS && padding < name##2x##h::COLS) {                          \
        for(uint32_t i = padding; i < name##2x##h::COLS; i++) {                                                     \
            res._cols[i][i] = type(1);                                                                              \
        }                                                                                                           \
    }                                                                                                               \
    return res;                                                                                                     \
}

#define _prvl_IMPL_MAT3XN(type, name, vecname, h)                                                                                               \
constexpr struct name##3x##h prvl::name##3x##h() noexcept {                                                                                     \
    return (struct name##3x##h){prvl::vecname##h(), prvl::vecname##h(), prvl::vecname##h()};                                                    \
}                                                                                                                                               \
constexpr struct name##3x##h prvl::name##3x##h(const struct vecname##h& a, const struct vecname##h& b, const struct vecname##h& c) noexcept {   \
    return (struct name##3x##h){a, b, c};                                                                                                       \
}                                                                                                                                               \
constexpr struct name##3x##h prvl::name##3x##h(const struct quat& q) noexcept {                                                                 \
    float xx = q.x * q.x;                                                                                                                       \
    float xy = q.x * q.y;                                                                                                                       \
    float xz = q.x * q.z;                                                                                                                       \
    float xw = q.x * q.w;                                                                                                                       \
    float yy = q.y * q.y;                                                                                                                       \
    float yz = q.y * q.z;                                                                                                                       \
    float yw = q.y * q.w;                                                                                                                       \
    float zz = q.z * q.z;                                                                                                                       \
    float zw = q.z * q.w;                                                                                                                       \
    struct vec3 other[]{                                                                                                                        \
        prvl::vec3(                                                                                                                             \
            1.0f - 2.0f * (yy + zz),                                                                                                            \
            2.0f * (xy + zw),                                                                                                                   \
            2.0f * (xz - yw)                                                                                                                    \
        ),                                                                                                                                      \
        prvl::vec3(                                                                                                                             \
            2.0f * (xy - zw),                                                                                                                   \
            1.0f - 2.0f * (xx + zz),                                                                                                            \
            2.0f * (yz + xw)                                                                                                                    \
        ),                                                                                                                                      \
        prvl::vec3(                                                                                                                             \
            2.0f * (xz + yw),                                                                                                                   \
            2.0f * (yz - xw),                                                                                                                   \
            1.0f - 2.0f * (xx + yy)                                                                                                             \
        )                                                                                                                                       \
    };                                                                                                                                          \
    struct name##3x##h res;                                                                                                                     \
    if constexpr (name##3x##h::ROWS == 3u) {                                                                                                    \
        for (uint32_t c = 0u; c < 3u; ++c) {                                                                                                    \
            for (uint32_t r = 0u; r < 3u; ++r) {                                                                                                \
                res._cols[c][r] = static_cast<type>(other[c][r]);                                                                               \
            }                                                                                                                                   \
        }                                                                                                                                       \
    } else {                                                                                                                                    \
        constexpr uint32_t copyRows = (name##3x##h::ROWS < 3u) ? name##3x##h::ROWS : 3u;                                                        \
        for (uint32_t c = 0u; c < 3u; ++c) {                                                                                                    \
            for (uint32_t r = 0u; r < copyRows; ++r) {                                                                                          \
                res._cols[c][r] = static_cast<type>(other[c][r]);                                                                               \
            }                                                                                                                                   \
        }                                                                                                                                       \
        if constexpr (name##3x##h::ROWS > 3u) {                                                                                                 \
            for (uint32_t c = 0u; c < 3u; ++c) {                                                                                                \
                for (uint32_t r = 3u; r < name##3x##h::ROWS; ++r) {                                                                             \
                    res._cols[c][r] = type(0);                                                                                                  \
                }                                                                                                                               \
            }                                                                                                                                   \
        }                                                                                                                                       \
    }                                                                                                                                           \
    return res;                                                                                                                                 \
}                                                                                                                                               \
constexpr struct name##3x##h prvl::name##3x##h(const struct dquat& q) noexcept {                                                                \
    double xx = q.x * q.x;                                                                                                                      \
    double xy = q.x * q.y;                                                                                                                      \
    double xz = q.x * q.z;                                                                                                                      \
    double xw = q.x * q.w;                                                                                                                      \
    double yy = q.y * q.y;                                                                                                                      \
    double yz = q.y * q.z;                                                                                                                      \
    double yw = q.y * q.w;                                                                                                                      \
    double zz = q.z * q.z;                                                                                                                      \
    double zw = q.z * q.w;                                                                                                                      \
    struct dvec3 other[]{                                                                                                                       \
        prvl::dvec3(                                                                                                                            \
            1.0 - 2.0 * (yy + zz),                                                                                                              \
            2.0 * (xy + zw),                                                                                                                    \
            2.0 * (xz - yw)                                                                                                                     \
        ),                                                                                                                                      \
        prvl::dvec3(                                                                                                                            \
            2.0 * (xy - zw),                                                                                                                    \
            1.0 - 2.0 * (xx + zz),                                                                                                              \
            2.0 * (yz + xw)                                                                                                                     \
        ),                                                                                                                                      \
        prvl::dvec3(                                                                                                                            \
            2.0 * (xz + yw),                                                                                                                    \
            2.0 * (yz - xw),                                                                                                                    \
            1.0 - 2.0 * (xx + yy)                                                                                                               \
        )                                                                                                                                       \
    };                                                                                                                                          \
    struct name##3x##h res;                                                                                                                     \
    if constexpr (name##3x##h::ROWS == 3u) {                                                                                                    \
        for (uint32_t c = 0u; c < 3u; ++c) {                                                                                                    \
            for (uint32_t r = 0u; r < 3u; ++r) {                                                                                                \
                res._cols[c][r] = static_cast<type>(other[c][r]);                                                                               \
            }                                                                                                                                   \
        }                                                                                                                                       \
    } else {                                                                                                                                    \
        constexpr uint32_t copyRows = (name##3x##h::ROWS < 3u) ? name##3x##h::ROWS : 3u;                                                        \
        for (uint32_t c = 0u; c < 3u; ++c) {                                                                                                    \
            for (uint32_t r = 0u; r < copyRows; ++r) {                                                                                          \
                res._cols[c][r] = static_cast<type>(other[c][r]);                                                                               \
            }                                                                                                                                   \
        }                                                                                                                                       \
        if constexpr (name##3x##h::ROWS > 3u) {                                                                                                 \
            for (uint32_t c = 0u; c < 3u; ++c) {                                                                                                \
                for (uint32_t r = 3u; r < name##3x##h::ROWS; ++r) {                                                                             \
                    res._cols[c][r] = type(0);                                                                                                  \
                }                                                                                                                               \
            }                                                                                                                                   \
        }                                                                                                                                       \
    }                                                                                                                                           \
    return res;                                                                                                                                 \
}                                                                                                                                               \
template<typename Other, typename>                                                                                                              \
constexpr struct name##3x##h prvl::name##3x##h(const Other& other) noexcept {                                                                   \
    constexpr uint32_t copyCols = (name##3x##h::COLS < Other::COLS) ? name##3x##h::COLS : Other::COLS;                                          \
    constexpr uint32_t copyRows = (name##3x##h::ROWS < Other::ROWS) ? name##3x##h::ROWS : Other::ROWS;                                          \
    struct name##3x##h res;                                                                                                                     \
    for (uint32_t c = 0u; c < copyCols; ++c) {                                                                                                  \
        for (uint32_t r = 0u; r < copyRows; ++r) {                                                                                              \
            res._cols[c][r] = other[c][r];                                                                                                      \
        }                                                                                                                                       \
    }                                                                                                                                           \
    if constexpr (name##3x##h::COLS > copyCols) {                                                                                               \
        for (uint32_t c = copyCols; c < name##3x##h::COLS; ++c) {                                                                               \
            res._cols[c] = prvl::vecname##h();                                                                                                  \
        }                                                                                                                                       \
    }                                                                                                                                           \
    if constexpr (name##3x##h::ROWS > copyRows) {                                                                                               \
        for (uint32_t c = 0u; c < copyCols; ++c) {                                                                                              \
            for (uint32_t r = copyRows; r < name##3x##h::ROWS; ++r) {                                                                           \
                res._cols[c][r] = type(0);                                                                                                      \
            }                                                                                                                                   \
        }                                                                                                                                       \
    }                                                                                                                                           \
    constexpr uint32_t padding = (copyCols < copyRows) ? copyCols : copyRows;                                                                   \
    if constexpr (name##3x##h::COLS == name##3x##h::ROWS && padding < name##3x##h::COLS) {                                                      \
        for(uint32_t i = padding; i < name##3x##h::COLS; i++) {                                                                                 \
            res._cols[i][i] = type(1);                                                                                                          \
        }                                                                                                                                       \
    }                                                                                                                                           \
    return res;                                                                                                                                 \
}

#define _prvl_IMPL_MAT4XN(type, name, vecname, h)                                                                                                                           \
constexpr struct name##4x##h prvl::name##4x##h() noexcept {                                                                                                                 \
    return (struct name##4x##h){prvl::vecname##h(), prvl::vecname##h(), prvl::vecname##h(), prvl::vecname##h()};                                                            \
}                                                                                                                                                                           \
constexpr struct name##4x##h prvl::name##4x##h(const struct vecname##h& a, const struct vecname##h& b, const struct vecname##h& c, const struct vecname##h& d) noexcept {   \
    return (struct name##4x##h){a, b, c, d};                                                                                                                                \
}                                                                                                                                                                           \
template<typename Other, typename>                                                                                                                                          \
constexpr struct name##4x##h prvl::name##4x##h(const Other& other) noexcept {                                                                                               \
    constexpr uint32_t copyCols = (name##4x##h::COLS < Other::COLS) ? name##4x##h::COLS : Other::COLS;                                                                      \
    constexpr uint32_t copyRows = (name##4x##h::ROWS < Other::ROWS) ? name##4x##h::ROWS : Other::ROWS;                                                                      \
    struct name##4x##h res;                                                                                                                                                 \
    for (uint32_t c = 0u; c < copyCols; ++c) {                                                                                                                              \
        for (uint32_t r = 0u; r < copyRows; ++r) {                                                                                                                          \
            res._cols[c][r] = other[c][r];                                                                                                                                  \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
    if constexpr (name##4x##h::COLS > copyCols) {                                                                                                                           \
        for (uint32_t c = copyCols; c < name##4x##h::COLS; ++c) {                                                                                                           \
            res._cols[c] = prvl::vecname##h();                                                                                                                              \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
    if constexpr (name##4x##h::ROWS > copyRows) {                                                                                                                           \
        for (uint32_t c = 0u; c < copyCols; ++c) {                                                                                                                          \
            for (uint32_t r = copyRows; r < name##4x##h::ROWS; ++r) {                                                                                                       \
                res._cols[c][r] = type(0);                                                                                                                                  \
            }                                                                                                                                                               \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
    constexpr uint32_t padding = (copyCols < copyRows) ? copyCols : copyRows;                                                                                               \
    if constexpr (name##4x##h::COLS == name##4x##h::ROWS && padding < name##4x##h::COLS) {                                                                                  \
        for(uint32_t i = padding; i < name##4x##h::COLS; i++) {                                                                                                             \
            res._cols[i][i] = type(1);                                                                                                                                      \
        }                                                                                                                                                                   \
    }                                                                                                                                                                       \
    return res;                                                                                                                                                             \
}

#define _prvl_IMPL_MAT_SQUARE(type, name, vecname)                                                                                                                  \
namespace prvl {                                                                                                                                                    \
    constexpr struct name##2x2 name##2() noexcept {                                                                                                                 \
        return name##2x2();                                                                                                                                         \
    }                                                                                                                                                               \
    constexpr struct name##2x2 name##2(const struct vecname##2& a, const struct vecname##2& b) noexcept {                                                           \
        return name##2x2(a, b);                                                                                                                                     \
    }                                                                                                                                                               \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                                                                                  \
    constexpr struct name##2x2 name##2(const Other& other) noexcept {                                                                                               \
        return name##2x2(other);                                                                                                                                    \
    }                                                                                                                                                               \
    constexpr struct name##3x3 name##3() noexcept {                                                                                                                 \
        return name##3x3();                                                                                                                                         \
    }                                                                                                                                                               \
    constexpr struct name##3x3 name##3(const struct vecname##3& a, const struct vecname##3& b, const struct vecname##3& c) noexcept {                               \
        return name##3x3(a, b, c);                                                                                                                                  \
    }                                                                                                                                                               \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                                                                                  \
    constexpr struct name##3x3 name##3(const Other& other) noexcept {                                                                                               \
        return name##3x3(other);                                                                                                                                    \
    }                                                                                                                                                               \
    constexpr struct name##3x3 name##3(const struct quat& q) noexcept {                                                                                             \
        return name##3x3(q);                                                                                                                                        \
    }                                                                                                                                                               \
    constexpr struct name##3x3 name##3(const struct dquat& q) noexcept {                                                                                            \
        return name##3x3(q);                                                                                                                                        \
    }                                                                                                                                                               \
    constexpr struct name##4x4 name##4() noexcept {                                                                                                                 \
        return name##4x4();                                                                                                                                         \
    }                                                                                                                                                               \
    constexpr struct name##4x4 name##4(const struct vecname##4& a, const struct vecname##4& b, const struct vecname##4& c, const struct vecname##4& d) noexcept {   \
        return name##4x4(a, b, c, d);                                                                                                                               \
    }                                                                                                                                                               \
    template<typename Other, typename = std::enable_if_t<is_matrix<Other>::value>>                                                                                  \
    constexpr struct name##4x4 name##4(const Other& other) noexcept {                                                                                               \
        return name##4x4(other);                                                                                                                                    \
    }                                                                                                                                                               \
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
using name##4 = name##4x4;                  \
_prvl_IMPL_MAT_SQUARE(type, name, vecname)  \


#define _prvl_MAT2_OP(name, h)                                                          \
constexpr name##2x##h operator+(const name##2x##h& a, const name##2x##h& b) noexcept {  \
    return prvl::name##2x##h(a[0] + b[0], a[1] + b[1]);                                 \
}                                                                                       \
constexpr name##2x##h operator-(const name##2x##h& a, const name##2x##h& b) noexcept {  \
    return prvl::name##2x##h(a[0] - b[0], a[1] - b[1]);                                 \
}                                                                                       \
constexpr name##2x##h& operator+=(name##2x##h& a, const name##2x##h& b) noexcept {      \
    a[0] += b[0];                                                                       \
    a[1] += b[1];                                                                       \
    return a;                                                                           \
}                                                                                       \
constexpr name##2x##h& operator-=(name##2x##h& a, const name##2x##h& b) noexcept {      \
    a[0] -= b[0];                                                                       \
    a[1] -= b[1];                                                                       \
    return a;                                                                           \
}

#define _prvl_MAT3_OP(name, h)                                                          \
constexpr name##3x##h operator+(const name##3x##h& a, const name##3x##h& b) noexcept {  \
    return prvl::name##3x##h(a[0] + b[0], a[1] + b[1], a[2] + b[2]);                    \
}                                                                                       \
constexpr name##3x##h operator-(const name##3x##h& a, const name##3x##h& b) noexcept {  \
    return prvl::name##3x##h(a[0] - b[0], a[1] - b[1], a[2] - b[2]);                    \
}                                                                                       \
constexpr name##3x##h& operator+=(name##3x##h& a, const name##3x##h& b) noexcept {      \
    a[0] += b[0];                                                                       \
    a[1] += b[1];                                                                       \
    a[2] += b[2];                                                                       \
    return a;                                                                           \
}                                                                                       \
constexpr name##3x##h& operator-=(name##3x##h& a, const name##3x##h& b) noexcept {      \
    a[0] -= b[0];                                                                       \
    a[1] -= b[1];                                                                       \
    a[2] -= b[2];                                                                       \
    return a;                                                                           \
}

#define _prvl_MAT4_OP(name, h)                                                          \
constexpr name##4x##h operator+(const name##4x##h& a, const name##4x##h& b) noexcept {  \
    return prvl::name##4x##h(a[0] + b[0], a[1] + b[1], a[2] + b[2], a[3] + b[3]);       \
}                                                                                       \
constexpr name##4x##h operator-(const name##4x##h& a, const name##4x##h& b) noexcept {  \
    return prvl::name##4x##h(a[0] - b[0], a[1] - b[1], a[2] - b[2], a[3] - b[3]);       \
}                                                                                       \
constexpr name##4x##h& operator+=(name##4x##h& a, const name##4x##h& b) noexcept {      \
    a[0] += b[0];                                                                       \
    a[1] += b[1];                                                                       \
    a[2] += b[2];                                                                       \
    a[3] += b[3];                                                                       \
    return a;                                                                           \
}                                                                                       \
constexpr name##4x##h& operator-=(name##4x##h& a, const name##4x##h& b) noexcept {      \
    a[0] -= b[0];                                                                       \
    a[1] -= b[1];                                                                       \
    a[2] -= b[2];                                                                       \
    a[3] -= b[3];                                                                       \
    return a;                                                                           \
}

#define _prvl_MAT_MUL(type, name, m, n, p)                                                      \
constexpr name##p##x##m operator*(const name##n##x##m& a, const name##p##x##n& b) noexcept {    \
    name##p##x##m r;                                                                            \
    _matmul<type, name##n##x##m, name##p##x##n, name##p##x##m>(a, b, r);                        \
    return r;                                                                                   \
}

#define _prvl_MAT_MOD_MUL(name, n)                                      \
constexpr name##n& operator*=(name##n& a, const name##n& b) noexcept {  \
    a = a * b;                                                          \
    return a;                                                           \
}

#define _prvl_MAT_VEC_MUL2(name, vecname, h)                                            \
constexpr vecname##h operator*(const name##2x##h& a, const vecname##2& b) noexcept {    \
    return a[0] * b.x + a[1] * b.y;                                                     \
}                                                                                       \
constexpr vecname##h operator*(const vecname##2& a, const name##2x##h& b) noexcept {    \
    return b[0] * a.x + b[1] * a.y;                                                     \
}

#define _prvl_MAT_VEC_MUL3(name, vecname, h)                                            \
constexpr vecname##h operator*(const name##3x##h& a, const vecname##3& b) noexcept {    \
    return a[0] * b.x + a[1] * b.y + a[2] * b.z;                                        \
}                                                                                       \
constexpr vecname##h operator*(const vecname##3& a, const name##3x##h& b) noexcept {    \
    return b[0] * a.x + b[1] * a.y + b[2] * a.z;                                        \
}

#define _prvl_MAT_VEC_MUL4(name, vecname, h)                                            \
constexpr vecname##h operator*(const name##4x##h& a, const vecname##4& b) noexcept {    \
    return a[0] * b.x + a[1] * b.y + a[2] * b.z + a[3] * b.w;                           \
}                                                                                       \
constexpr vecname##h operator*(const vecname##4& a, const name##4x##h& b) noexcept {    \
    return b[0] * a.x + b[1] * a.y + b[2] * a.z + b[3] * a.w;                           \
}

#define _prvl_MAT_SCALAR_MUL2(type, name, h)                                    \
constexpr name##2x##h operator*(const name##2x##h& a, const type b) noexcept {  \
    name##2x##h r;                                                              \
    type* rd = r.data();                                                        \
    const type* ad = a.data();                                                  \
    for(int i = 0; i < 2 * h; i++) {                                            \
        rd[i] = ad[i] * b;                                                      \
    }                                                                           \
    return r;                                                                   \
}                                                                               \
constexpr name##2x##h operator*(const type a, const name##2x##h& b) noexcept {  \
    name##2x##h r;                                                              \
    type* rd = r.data();                                                        \
    const type* bd = b.data();                                                  \
    for(int i = 0; i < 2 * h; i++) {                                            \
        rd[i] = bd[i] * a;                                                      \
    }                                                                           \
    return r;                                                                   \
}

#define _prvl_MAT_SCALAR_MUL3(type, name, h)                                    \
constexpr name##3x##h operator*(const name##3x##h& a, const type b) noexcept {  \
    name##3x##h r;                                                              \
    type* rd = r.data();                                                        \
    const type* ad = a.data();                                                  \
    for(int i = 0; i < 3 * h; i++) {                                            \
        rd[i] = ad[i] * b;                                                      \
    }                                                                           \
    return r;                                                                   \
}                                                                               \
constexpr name##3x##h operator*(const type a, const name##3x##h& b) noexcept {  \
    name##3x##h r;                                                              \
    type* rd = r.data();                                                        \
    const type* bd = b.data();                                                  \
    for(int i = 0; i < 3 * h; i++) {                                            \
        rd[i] = bd[i] * a;                                                      \
    }                                                                           \
    return r;                                                                   \
}

#define _prvl_MAT_SCALAR_MUL4(type, name, h)                                    \
constexpr name##4x##h operator*(const name##4x##h& a, const type b) noexcept {  \
    name##4x##h r;                                                              \
    type* rd = r.data();                                                        \
    const type* ad = a.data();                                                  \
    for(int i = 0; i < 4 * h; i++) {                                            \
        rd[i] = ad[i] * b;                                                      \
    }                                                                           \
    return r;                                                                   \
}                                                                               \
constexpr name##4x##h operator*(const type a, const name##4x##h& b) noexcept {  \
    name##4x##h r;                                                              \
    type* rd = r.data();                                                        \
    const type* bd = b.data();                                                  \
    for(int i = 0; i < 4 * h; i++) {                                            \
        rd[i] = bd[i] * a;                                                      \
    }                                                                           \
    return r;                                                                   \
}

#define _prvl_MAT_TRANSPOSE(type, name, w, h)                           \
constexpr name##h##x##w transpose(const name##w##x##h& m) noexcept {    \
    name##h##x##w r;                                                    \
    _transpose<type, w, h>(m.data(), r.data());                         \
    return r;                                                           \
}

#define _prvl_MAT_DETERMINANT(type, name)               \
constexpr type determinant(const name##2& m) noexcept { \
    const type* d = m.data();                           \
    return d[0] * d[3] - d[1] * d[2];                   \
}                                                       \
constexpr type determinant(const name##3& m) noexcept { \
    const type* d = m.data();                           \
    return d[0] * (d[4] * d[8] - d[5] * d[7])           \
         - d[1] * (d[3] * d[8] - d[5] * d[6])           \
         + d[2] * (d[3] * d[7] - d[4] * d[6]);          \
}                                                       \
constexpr type determinant(const name##4& m) noexcept { \
    const type* d = m.data();                           \
    const type A = d[10] * d[15] - d[11] * d[14];       \
    const type B = d[6] * d[11] - d[7] * d[10];         \
    const type C = d[7] * d[14] - d[6] * d[15];         \
    const type D = d[4] * d[9] - d[5] * d[8];           \
    const type E = d[8] * d[13] - d[9] * d[12];         \
    const type F = d[5] * d[12] - d[4] * d[13];         \
    return d[0] * (A * d[5] +  B * d[13] + C * d[9])    \
         - d[1] * (A * d[4] +  B * d[12] + C * d[8])    \
         + d[2] * (D * d[15] + E * d[7] +  F * d[11])   \
         - d[3] * (D * d[14] + E * d[6] +  F * d[10]);  \
}

#define _prvl_MAT_INVERSE(type, name, vecname, unit)            \
constexpr name##2 inverse(const name##2& m) {                   \
    type det = unit / (m[0].x * m[1].y - m[0].y * m[1].x);      \
    name##2 r;                                                  \
    r[0].x =  m[1].y * det;                                     \
    r[0].y = -m[1].x * det;                                     \
    r[1].x = -m[0].y * det;                                     \
    r[1].y =  m[0].x * det;                                     \
    return r;                                                   \
}                                                               \
constexpr name##3 inverse(const name##3& m) {                   \
    const type A = m[1].y * m[2].z;                             \
    const type B = m[1].z * m[2].y;                             \
    const type C = m[1].x * m[2].z;                             \
    const type D = m[1].z * m[2].x;                             \
    const type E = m[1].x * m[2].y;                             \
    const type F = m[1].y * m[2].x;                             \
    type det = unit / (m[0].x * (A - B)                         \
                     - m[0].y * (C - D)                         \
                     + m[0].z * (E - F));                       \
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
constexpr name##4 inverse(const name##4& m) {                   \
    name##4 r;                                                  \
    type A = m[2][2] * m[3][3] - m[2][3] * m[3][2];             \
    type B = m[1][3] * m[3][2] - m[1][2] * m[3][3];             \
    type C = m[1][2] * m[2][3] - m[1][3] * m[2][2];             \
    type D = m[2][1] * m[3][3] - m[2][3] * m[3][1];             \
    type E = m[1][3] * m[3][1] - m[1][1] * m[3][3];             \
    type F = m[1][1] * m[2][3] - m[1][3] * m[2][1];             \
    type G = m[2][2] * m[3][1] - m[2][1] * m[3][2];             \
    type H = m[1][1] * m[3][2] - m[1][2] * m[3][1];             \
    type I = m[1][2] * m[2][1] - m[1][1] * m[2][2];             \
    type J = m[0][2] * m[3][3] - m[0][3] * m[3][2];             \
    type K = m[0][3] * m[2][2] - m[0][2] * m[2][3];             \
    type L = m[0][1] * m[3][3] - m[0][3] * m[3][1];             \
    type M = m[0][3] * m[2][1] - m[0][1] * m[2][3];             \
    type N = m[0][2] * m[3][1] - m[0][1] * m[3][2];             \
    type O = m[0][1] * m[2][2] - m[0][2] * m[2][1];             \
    type P = m[0][2] * m[1][3] - m[0][3] * m[1][2];             \
    type Q = m[0][1] * m[1][3] - m[0][3] * m[1][1];             \
    type R = m[0][2] * m[1][1] - m[0][1] * m[1][2];             \
    r[0][0] = m[1][1] * A                                       \
            + m[2][1] * B                                       \
            + m[3][1] * C;                                      \
    r[1][0] =-m[1][0] * A                                       \
            - m[2][0] * B                                       \
            - m[3][0] * C;                                      \
    r[2][0] = m[1][0] * D                                       \
            + m[2][0] * E                                       \
            + m[3][0] * F;                                      \
    r[3][0] = m[1][0] * G                                       \
            + m[2][0] * H                                       \
            + m[3][0] * I;                                      \
    r[0][1] =-m[0][1] * A                                       \
            + m[2][1] * J                                       \
            + m[3][1] * K;                                      \
    r[1][1] = m[0][0] * A                                       \
            - m[2][0] * J                                       \
            - m[3][0] * K;                                      \
    r[2][1] =-m[0][0] * D                                       \
            + m[2][0] * L                                       \
            + m[3][0] * M;                                      \
    r[3][1] =-m[0][0] * G                                       \
            + m[2][0] * N                                       \
            + m[3][0] * O;                                      \
    r[0][2] =-m[0][1] * B                                       \
            - m[1][1] * J                                       \
            + m[3][1] * P;                                      \
    r[1][2] = m[0][0] * B                                       \
            + m[1][0] * J                                       \
            - m[3][0] * P;                                      \
    r[2][2] =-m[0][0] * E                                       \
            - m[1][0] * L                                       \
            + m[3][0] * Q;                                      \
    r[3][2] =-m[0][0] * H                                       \
            - m[1][0] * N                                       \
            + m[3][0] * R;                                      \
    r[0][3] =-m[0][1] * C                                       \
            - m[1][1] * K                                       \
            - m[2][1] * P;                                      \
    r[1][3] = m[0][0] * C                                       \
            + m[1][0] * K                                       \
            + m[2][0] * P;                                      \
    r[2][3] =-m[0][0] * F                                       \
            - m[1][0] * M                                       \
            - m[2][0] * Q;                                      \
    r[3][3] =-m[0][0] * I                                       \
            - m[1][0] * O                                       \
            - m[2][0] * R;                                      \
    type det = unit / (m[0][0] * r[0][0] + m[0][1] * r[1][0]    \
                     + m[0][2] * r[2][0] + m[0][3] * r[3][0]);  \
    r[0][0] *= det;                                             \
    r[0][1] *= det;                                             \
    r[0][2] *= det;                                             \
    r[0][3] *= det;                                             \
    r[1][0] *= det;                                             \
    r[1][1] *= det;                                             \
    r[1][2] *= det;                                             \
    r[1][3] *= det;                                             \
    r[2][0] *= det;                                             \
    r[2][1] *= det;                                             \
    r[2][2] *= det;                                             \
    r[2][3] *= det;                                             \
    r[3][0] *= det;                                             \
    r[3][1] *= det;                                             \
    r[3][2] *= det;                                             \
    r[3][3] *= det;                                             \
    return r;                                                   \
}

#define _prvl_MAT_NORMALIZE(type, name)                                                 \
constexpr name##2 normalize(const name##2& m) noexcept {                                \
    type det = determinant(m);                                                          \
    type scalar = static_cast<type>(1.0 / sqrt(static_cast<double>(det)));              \
    return m * scalar;                                                                  \
}                                                                                       \
constexpr name##3 normalize(const name##3& m) noexcept {                                \
    type det = determinant(m);                                                          \
    type scalar = static_cast<type>(1.0 / pow(static_cast<double>(det), 1.0 / 3.0));    \
    return m * scalar;                                                                  \
}                                                                                       \
constexpr name##4 normalize(const name##4& m) noexcept {                                \
    type det = determinant(m);                                                          \
    type scalar = static_cast<type>(1.0 / pow(static_cast<double>(det), 0.25));         \
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
constexpr name##2 diag(const vecname##2& v) noexcept {              \
    name##2 r = prvl::name##2();                                    \
    r[0].x = v.x;                                                   \
    r[1].y = v.y;                                                   \
    return r;                                                       \
}                                                                   \
constexpr name##3 diag(const vecname##3& v) noexcept {              \
    name##3 r = prvl::name##3();                                    \
    r[0].x = v.x;                                                   \
    r[1].y = v.y;                                                   \
    r[2].z = v.z;                                                   \
    return r;                                                       \
}                                                                   \
constexpr name##4 diag(const vecname##4& v) noexcept {              \
    name##4 r = prvl::name##4();                                    \
    r[0].x = v.x;                                                   \
    r[1].y = v.y;                                                   \
    r[2].z = v.z;                                                   \
    r[3].w = v.w;                                                   \
    return r;                                                       \
}
#define _prvl_MAT_TOEPLITZ(name, vecname)                               \
constexpr name##2 toeplitz(const vecname##2& v) noexcept {              \
    name##2 r = prvl::name##2();                                        \
    r[0] = v;                                                           \
    r[1].y = v.x;                                                       \
    return r;                                                           \
}                                                                       \
constexpr name##3 toeplitz(const vecname##3& v) noexcept {              \
    name##3 r = prvl::name##3();                                        \
    r[0] = v;                                                           \
    r[1].y = v.x;                                                       \
    r[1].z = v.y;                                                       \
    r[2].z = v.x;                                                       \
    return r;                                                           \
}                                                                       \
constexpr name##4 toeplitz(const vecname##4& v) noexcept {              \
    name##4 r = prvl::name##4();                                        \
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

template<typename T, typename MatA, typename MatB, typename MatC>
constexpr void _matmul(const MatA& a, const MatB& b, MatC& out) noexcept {
    // a: M rows, N cols
    // b: N rows, P cols
    // out: M rows, P cols
    for (uint32_t col = 0u; col < MatC::COLS; col++) {
        for (uint32_t row = 0u; row < MatC::ROWS; row++) {
            T sum{};
            for (uint32_t k = 0u; k < MatA::COLS; k++) {
                sum += a[k][row] * b[col][k];
            }
            out[col][row] = sum;
        }
    }
}

template<typename T, int M, int N>
constexpr void _transpose(const T* m, T* out) noexcept {
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

_prvl_MAT_INVERSE(float, mat, vec, 1.0f);
_prvl_MAT_INVERSE(double, dmat, dvec, 1.0);

constexpr mat3 operator+(const mat3& a, const quat& b) {
    return a + prvl::mat3(b);
}

constexpr mat3& operator+=(mat3& a, const quat& b) {
    a += prvl::mat3(b);
    return a;
}

constexpr mat3 operator-(const mat3& a, const quat& b) {
    return a - prvl::mat3(b);
}

constexpr mat3& operator-=(mat3& a, const quat& b) {
    a -= prvl::mat3(b);
    return a;
}

constexpr mat3 operator*(const mat3& a, const quat& b) {
    return a * prvl::mat3(b);
}

constexpr mat3& operator*=(mat3& a, const quat& b) {
    a *= prvl::mat3(b);
    return a;
}

constexpr dmat3 operator+(const dmat3& a, const dquat& b) {
    return a + prvl::dmat3(b);
}

constexpr dmat3& operator+=(dmat3& a, const dquat& b) {
    a += prvl::dmat3(b);
    return a;
}

constexpr dmat3 operator-(const dmat3& a, const dquat& b) {
    return a - prvl::dmat3(b);
}

constexpr dmat3& operator-=(dmat3& a, const dquat& b) {
    a -= prvl::dmat3(b);
    return a;
}

constexpr dmat3 operator*(const dmat3& a, const dquat& b) {
    return a * prvl::dmat3(b);
}

constexpr dmat3& operator*=(dmat3& a, const dquat& b) {
    a *= prvl::dmat3(b);
    return a;
}

constexpr mat3 rotateX(float ang) {
    float sin = sinf(ang);
    float cos = cosf(ang);
    mat3 r = prvl::mat3();
    r[0][0] = 1.0f;
    r[1][1] = cos;
    r[1][2] = sin;
    r[2][1] = -sin;
    r[2][2] = cos;
    return r;
}

constexpr mat3 rotateY(float ang) {
    float sin = sinf(ang);
    float cos = cosf(ang);
    mat3 r = prvl::mat3();
    r[1][1] = 1.0f;
    r[0][0] = cos;
    r[0][2] = sin;
    r[2][0] = -sin;
    r[2][2] = cos;
    return r;
}

constexpr mat3 rotateZ(float ang) {
    float sin = sinf(ang);
    float cos = cosf(ang);
    mat3 r = prvl::mat3();
    r[2][2] = 1.0f;
    r[0][0] = cos;
    r[0][1] = sin;
    r[1][0] = -sin;
    r[1][1] = cos;
    return r;
}

constexpr mat3 lookat(const vec3& from, const vec3& to) {
    vec3 f = normalize(from - to);
    vec3 r = normalize(prvl::vec3(-f.z, 0.0f, f.x));
    vec3 u = prvl::vec3(-r.z * f.y, r.z * f.x - r.x * f.z, r.x * f.y);
    return transpose(prvl::mat3(r, u, f));
}


constexpr mat4 perspectiveProjection(const float fov, const float aspect, const float nearZ, const float farZ) {
    mat4 r = prvl::mat4();
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

constexpr mat4 reverseZPerspectiveProjection(const float fov, const float aspect, const float nearZ) {
    mat4 r = prvl::mat4();
    const float yScale = 1.0f / tanf(fov * 0.5f);
    const float xScale = yScale / aspect;
    r[0][0] = xScale;
    r[1][1] = yScale;
    r[2][3] = -1.0f;
    r[3][2] = nearZ;
    return r;
}

constexpr void setPlane(const float a, const float b, const float c, const float d, const uint32_t idx, float* planes, bool* planeDirs) {
    float mul = 1.0f / sqrtf(a * a + b * b + c * c);
    planes[idx] = a * mul;
    planeDirs[idx] = a >= 0.0f;
    planes[idx + 1] = b * mul;
    planeDirs[idx + 1] = b >= 0.0f;
    planes[idx + 2] = c * mul;
    planeDirs[idx + 2] = c >= 0.0f;
    planes[idx + 3] = d * mul;
}

constexpr void getViewFrustumPlanes(const mat4& proj, const mat4& cam, float* planes, bool* planeDirs) {
    mat4 r = proj * cam;
    setPlane(r[0][3] + r[0][0], r[1][3] + r[1][0], r[2][3] + r[2][0], r[3][3] + r[3][0], 0u, planes, planeDirs);
    setPlane(r[0][3] - r[0][0], r[1][3] - r[1][0], r[2][3] - r[2][0], r[3][3] - r[3][0], 4u, planes, planeDirs);
    setPlane(r[0][3] + r[0][1], r[1][3] + r[1][1], r[2][3] + r[2][1], r[3][3] + r[3][1], 8u, planes, planeDirs);
    setPlane(r[0][3] - r[0][1], r[1][3] - r[1][1], r[2][3] - r[2][1], r[3][3] - r[3][1], 12u, planes, planeDirs);
    setPlane(r[0][3] + r[0][2], r[1][3] + r[1][2], r[2][3] + r[2][2], r[3][3] + r[3][2], 16u, planes, planeDirs);
    //setPlane(r[0][3] - r[0][2], r[1][3] - r[1][2], r[2][3] - r[2][2], r[3][3] - r[3][2], 20u, planes, planeDirs);
}

constexpr bool isPointVisible(const float* planes, const vec3& p) {
    for (int i = 0; i < 5; i++) {
        int idx = i << 2;
        if (planes[idx] * p.x + planes[idx + 1] * p.y + planes[idx + 2] * p.z + planes[idx + 3] < 0.0f) {
            return false;
        }
    }
    return true;
}

constexpr bool isAABBVisible(const float* planes, const bool* planeDirs, const vec3& min, const vec3& max) {
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

constexpr void isMultiAABBVisible(const float* planes, const bool* planeDirs, const float* minX, const float* minY, const float* minZ, const float* maxX, const float* maxY, const float* maxZ, bool* out, const int count) {
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
