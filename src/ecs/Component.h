#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#pragma once

#include <type_traits>

template<typename T>
concept Component = std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>;

#endif