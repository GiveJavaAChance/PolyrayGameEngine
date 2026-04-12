#ifndef INPUT_H_INCLUDED
#define INPUT_H_INCLUDED

#pragma once

#include <unordered_set>

namespace Input {
    namespace Internal {
        inline std::unordered_set<int32_t> keys;

        inline void setKey(int32_t key, bool active) {
            if (active) {
                keys.insert(key);
            } else {
                keys.erase(key);
            }
        }
    }

    using namespace Internal;

    inline bool getKey(int32_t key) {
        return keys.count(key) != 0;
    }

    inline std::unordered_set<int32_t>& getActiveKeys() {
        return keys;
    }
}

#endif
