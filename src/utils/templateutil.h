#ifndef TEMPLATEUTIL_H_INCLUDED
#define TEMPLATEUTIL_H_INCLUDED

#pragma once

#include <utility>

template <typename... Ts>
struct type_list {};

template <typename T, std::size_t N, typename Seq>
struct repeat_impl;

template <typename A, typename B>
struct always_t {
    using type = A;
};

template <typename T, std::size_t N, std::size_t... I>
struct repeat_impl<T, N, std::index_sequence<I...>> {
    using type = type_list<always_t<T, std::integral_constant<std::size_t, I>>::type...>;
};

template <typename A, typename B>
struct concat;

template <typename A, typename B>
struct concat<A, type_list<B>> {
    using type = type_list<A, B>;
};

template <typename... A, typename B>
struct concat<type_list<A...>, B> {
    using type = type_list<A..., B>;
};

template <typename A, typename... B>
struct concat<A, type_list<B...>> {
    using type = type_list<A, B...>;
};

template <typename... A, typename... B>
struct concat<type_list<A...>, type_list<B...>> {
    using type = type_list<A..., B...>;
};

template <template <typename...> class C, typename List>
struct apply;

template <template <typename...> class C, typename... Ts>
struct apply<C, type_list<Ts...>> {
    using type = C<Ts...>;
};

template <typename T, std::size_t N>
using repeat_t = typename repeat_impl<T, N, std::make_index_sequence<N>>::type;

template <typename A, typename B>
using concat_t = typename concat<A, B>::type;

template <template <typename...> class C, typename List>
using apply_t = typename apply<C, List>::type;

#endif