#ifndef COMPONENTMETADATA_H_INCLUDED
#define COMPONENTMETADATA_H_INCLUDED

#pragma once

#include <ecs/Component.h>
#include <ecs/Export.h>
#include <serial/Serial.h>
#include <structure/DynamicArray.h>
#include <typereg.h>

struct World;
struct ByteReader;
struct ByteWriter;

struct ComponentMetadata {
private:
    TYPE_REGISTRY(ComponentTypes)

    template <Component T>
    constexpr static const Export* getExports() {
        if constexpr (HasExport<T>) {
            return ExportInfo<T>::__export__;
        } else {
            return nullptr;
        }
    }

    template <Component T>
    constexpr static uint32_t getExportCount() {
        if constexpr (HasExport<T>) {
            return sizeof(ExportInfo<T>::__export__) / sizeof(Export);
        } else {
            return 0u;
        }
    }

public:
    uint32_t typeId;
    std::string_view name;

    uint32_t size;
    uint32_t alignment;

    void (*serializer)(World*, uint32_t, ByteWriter&);
    void (*deserializer)(World*, Entity&, ByteReader&);

    const Export* exports;
    uint32_t exportCount;

    template <Component T>
    inline static uint32_t typeOf() {
        return ComponentTypes::getTypeId<T>();
    }

    template <Component T>
    inline static ComponentMetadata get() {
        return ComponentMetadata{
            ComponentTypes::getTypeId<T>(),
            type_name_of<T>(),
            sizeof(T),
            alignof(T),
            &Serial<T>::serialize,
            &Serial<T>::deserialize,
            getExports<T>(),
            getExportCount<T>()
        };
    }

    constexpr static ComponentMetadata invalid() {
        return ComponentMetadata{UINT32_MAX, "", 0u, 0u, nullptr, nullptr, nullptr, 0u};
    }
};

#endif