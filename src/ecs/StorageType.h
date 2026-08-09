#ifndef STORAGETYPE_H_INCLUDED
#define STORAGETYPE_H_INCLUDED

#pragma once

#include <ecs/Storage.h>

template <typename T>
concept is_default_storage_v = requires { Storage<T>::__DEFAULT__; };

#endif