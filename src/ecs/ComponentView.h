#ifndef COMPONENTVIEW_H_INCLUDED
#define COMPONENTVIEW_H_INCLUDED

#pragma once

#include <cstdint>

#include <ecs/Storage.h>

template <typename T>
struct ComponentView {
private:
    using ReturnType = decltype(std::declval<Storage<T>>().get(0));

    static constexpr bool IsReference = std::is_reference_v<ReturnType>;

    using BaseType = std::remove_reference_t<ReturnType>;
    using ValueType = std::conditional_t<IsReference, BaseType*, BaseType>;

    ValueType value;

    Storage<T>* storage;
    uint32_t componentID;

public:
    ComponentView(Storage<T>* storage, uint32_t componentID) : storage(storage), componentID(componentID) {
        if constexpr (IsReference) {
            value = &storage->get(componentID);
        } else {
            value = storage->get(componentID);
        }
    }

    ~ComponentView() {
        if constexpr (!IsReference) {
            storage->set(componentID, std::move(value));
        }
    }

    inline uint32_t id() {
        return componentID;
    }

    operator BaseType&() {
        if constexpr (IsReference) {
            return *value;
        } else {
            return value;
        }
    }

    BaseType& operator*() {
        if constexpr (IsReference) {
            return *value;
        } else {
            return value;
        }
    }

    BaseType* operator->() {
        if constexpr (IsReference) {
            return value;
        } else {
            return &value;
        }
    }
};

#endif