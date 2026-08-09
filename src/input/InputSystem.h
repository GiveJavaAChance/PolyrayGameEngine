#ifndef INPUTSYSTEM_H_INCLUDED
#define INPUTSYSTEM_H_INCLUDED

#pragma once

#include <input/BindSet.h>
#include <cstdint>

namespace InputSystem {
    namespace {
        inline std::unordered_map<const char*, BindSet> bindSets;
        inline std::vector<BindSet*> bindStack;
    }

    inline BindSet& getOrCreateBindSet(const char* name) {
        auto it = bindSets.find(name);
        if (it != bindSets.end()) {
            return it->second;
        }
        bindSets[name] = BindSet(nullptr);
        return bindSets[name];
    }

    inline BindSet& getOrCreateBindSet(const char* name, const char* deriveFrom) {
        auto it = bindSets.find(name);
        if (it != bindSets.end()) {
            return it->second;
        }
        bindSets[name] = BindSet(&getOrCreateBindSet(deriveFrom));
        return bindSets[name];
    }

    inline void push(const char* name) {
        auto it = bindSets.find(name);
        if (it != bindSets.end()) {
            bindStack.push_back(&it->second);
            it->second.onPush();
        }
    }

    inline void pop() {
        if (!bindStack.empty()) {
            BindSet* binds = bindStack.back();
            bindStack.pop_back();
            binds->onPop();
        }
    }

    inline void mousePress(float x, float y, int32_t button) {
        if (!bindStack.empty()) {
            bindStack.back()->mousePress(x, y, button);
        }
    }

    inline void keyPress(int32_t key) {
        if (!bindStack.empty()) {
            bindStack.back()->keyPress(key);
        }
    }

    inline bool getKey(int32_t key) {
        return !bindStack.empty() && bindStack.back()->getKey(key);
    }

    inline bool isCurrent(const char* name) {
        auto it = bindSets.find(name);
        return !bindStack.empty() && it != bindSets.end() && bindStack.back() == &it->second;
    }

    inline bool isActive(const char* name) {
        auto it = bindSets.find(name);
        if (it == bindSets.end()) {
            return false;
        }
        for (auto& bind : bindStack) {
            if (bind == &it->second) {
                return true;
            }
        }
        return false;
    }
}

#endif
