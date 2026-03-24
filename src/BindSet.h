#ifndef BINDSET_H_INCLUDED
#define BINDSET_H_INCLUDED

#pragma once

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <initializer_list>

#include "Input.h"

struct BindSet {
private:
    BindSet* parent;
    std::unordered_map<int32_t, std::function<void(float, float)>> mouseBindings;
    std::unordered_map<int32_t, std::function<void()>> keyBindings;
    std::unordered_set<int32_t> allowedKeys;

    std::function<void()> onPushCallback;
    std::function<void()> onPopCallback;

public:
    BindSet(BindSet* set = nullptr) : parent(set) {
    }

    void setOnPush(std::function<void()> callback) {
        onPushCallback = std::move(callback);
    }

    void setOnPop(std::function<void()> callback) {
        onPopCallback = std::move(callback);
    }

    inline void onPush() {
        if(parent) {
            parent->onPush();
        }
        if(onPushCallback) {
            onPushCallback();
        }
    }

    inline void onPop() {
        if(parent) {
            parent->onPop();
        }
        if(onPopCallback) {
            onPopCallback();
        }
    }

    void setMouseBinding(int32_t button, std::function<void(float, float)> callback) {
        mouseBindings[button] = std::move(callback);
    }

    void setKeyBinding(int32_t key, std::function<void()> callback) {
        keyBindings[key] = std::move(callback);
    }

    void mousePress(float x, float y, int32_t button) {
        auto it = mouseBindings.find(button);
        if (it != mouseBindings.end()) {
            it->second(x, y);
        } else if (parent) {
            parent->mousePress(x, y, button);
        }
    }

    void keyPress(int32_t key) {
        auto it = keyBindings.find(key);
        if (it != keyBindings.end()) {
            it->second();
        } else if (parent) {
            parent->keyPress(key);
        }
    }

    void allowKeys(std::initializer_list<int32_t> keys) {
        for (int32_t key : keys) {
            allowedKeys.insert(key);
        }
    }

    void blockKeys(std::initializer_list<int32_t> keys) {
        for (int32_t key : keys) {
            allowedKeys.erase(key);
        }
    }

    bool getKey(int32_t key) const {
        return allowedKeys.count(key) && Input::getKey(key);
    }
};
#endif
