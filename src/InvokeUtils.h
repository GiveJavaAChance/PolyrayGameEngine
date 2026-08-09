#ifndef INVOKEUTILS_H_INCLUDED
#define INVOKEUTILS_H_INCLUDED

#pragma once

template <typename... Ts>
struct Invoke {
    template <typename T, void (T::*Func)(Ts...)>
    inline static void thunk(void* instance, Ts... args) {
        (reinterpret_cast<T*>(instance)->*Func)(args...);
    }

    template <typename T, typename Return, Return (T::*Func)(Ts...)>
    inline static Return thunkReturn(void* instance, Ts... args) {
        return (reinterpret_cast<T*>(instance)->*Func)(args...);
    }

    template <void (*Func)(Ts...)>
    inline static void wrapThunk(void* instance, Ts... args) {
        Func(args...);
    }

    template <typename Return, Return (*Func)(Ts...)>
    inline static Return wrapThunkReturn(void* instance, Ts... args) {
        return Func(args...);
    }
};

#endif