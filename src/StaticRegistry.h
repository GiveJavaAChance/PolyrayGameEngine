#ifndef STATICREGISTRY_H_INCLUDED
#define STATICREGISTRY_H_INCLUDED

#pragma once

#include <cstdint>
#include "DynamicArray.h"
#include "IDGenerator.h"


template<void(*move)(const uint32_t dst, const uint32_t src)>
struct StaticRegistry {
private:
    IDGenerator gen;
    uint32_t count;

public:
    DynamicArray<uint32_t> IDs;
    DynamicArray<uint32_t> locations;

    StaticRegistry() : count(0u) {
    }

    inline uint32_t create() {
        const uint32_t ID = gen.getNewID();
        locations.ensureCapacity(ID + 1u);
        locations[ID] = count++;
        IDs.add(ID);
        return ID;
    }

    inline void remove(const uint32_t ID) {
        gen.free(ID);
        const uint32_t location = locations[ID];
        const uint32_t end = --count;
        if(location != end) {
            const uint32_t endID = IDs[end];
            locations[endID] = location;
            IDs[location] = endID;
            move(location, end);
        }
        IDs.removeEnd(1u);
    }

    inline uint32_t size() const {
        return count;
    }

    inline uint32_t operator[](const uint32_t ID) const {
        return locations[ID];
    }

    inline IDGenerator::Iterator begin() {
        return gen.begin();
    }

    inline IDGenerator::Iterator end() {
        return gen.end();
    }
};

#endif
