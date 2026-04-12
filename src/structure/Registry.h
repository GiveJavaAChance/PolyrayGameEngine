#ifndef REGISTRY_H_INCLUDED
#define REGISTRY_H_INCLUDED

#pragma once

#include <cstdint>
#include <structure/DynamicArray.h>
#include <structure/IDGenerator.h>

struct Registry {
private:
    IDGenerator gen;
    uint32_t count;

public:
    DynamicArray<uint32_t> IDs;
    DynamicArray<uint32_t> locations;

    Registry() noexcept : count(0u) {
    }

    inline uint32_t create() {
        const uint32_t ID = gen.getNewID();
        locations.ensureCapacity(ID + 1u);
        locations[ID] = count++;
        IDs.add(ID);
        return ID;
    }

    inline bool remove(const uint32_t ID, uint32_t& loc) {
        gen.free(ID);
        const uint32_t location = locations[ID];
        const uint32_t end = --count;
        bool out = false;
        if(location != end) {
            const uint32_t endID = IDs[end];
            locations[endID] = location;
            IDs[location] = endID;
            loc = location;
            out = true;
        }
        IDs.removeEnd(1u);
        return out;
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
