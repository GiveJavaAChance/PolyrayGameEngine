#ifndef SOCKETHANDLE_H_INCLUDED
#define SOCKETHANDLE_H_INCLUDED

#pragma once

#include <cstdint>

#ifdef _WIN32

using SocketHandle = uintptr_t;

constexpr SocketHandle INVALID_SOCKET_HANDLE = (SocketHandle) (~0);

#else

using SocketHandle = int;

constexpr SocketHandle INVALID_SOCKET_HANDLE = -1;

#endif

#endif