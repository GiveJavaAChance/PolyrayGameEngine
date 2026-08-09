#ifndef COMPONENT_H_INCLUDED
#define COMPONENT_H_INCLUDED

#pragma once

#include <type_traits>

#include <ecs/StorageType.h>

template <typename T>
concept Component = (!is_default_storage_v<T>) || (std::is_trivially_copyable_v<T> && std::is_trivially_destructible_v<T>);

#endif
