#ifndef INPUTSYSTEM_H_INCLUDED
#define INPUTSYSTEM_H_INCLUDED

#pragma once

#include "BindSet.h"
#include <cstdint>

namespace InputSystem {
    namespace {
        inline std::unordered_map<const char*, BindSet> bindSets;
        inline std::vector<BindSet*> bindStack;
    }

    BindSet& getOrCreateBindSet(const char* name) {
        auto it = bindSets.find(name);
        if (it != bindSets.end()) {
            return it->second;
        }
        bindSets[name] = BindSet(nullptr);
        return bindSets[name];
    }

    BindSet& getOrCreateBindSet(const char* name, const char* deriveFrom) {
        auto it = bindSets.find(name);
        if (it != bindSets.end()) {
            return it->second;
        }
        bindSets[name] = BindSet(&getOrCreateBindSet(deriveFrom));
        return bindSets[name];
    }

    void push(const char* name) {
        auto it = bindSets.find(name);
        if (it != bindSets.end()) {
            bindStack.push_back(&it->second);
            it->second.onPush();
        }
    }

    void pop() {
        if (!bindStack.empty()) {
            BindSet* binds = bindStack.back();
            bindStack.pop_back();
            binds->onPop();
        }
    }

    void mousePress(float x, float y, int32_t button) {
        if (!bindStack.empty()) {
            bindStack.back()->mousePress(x, y, button);
        }
    }

    void keyPress(int32_t key) {
        if (!bindStack.empty()) {
            bindStack.back()->keyPress(key);
        }
    }

    bool getKey(int32_t key) {
        return !bindStack.empty() && bindStack.back()->getKey(key);
    }

    bool isCurrent(const char* name) {
        auto it = bindSets.find(name);
        return !bindStack.empty() && it != bindSets.end() && bindStack.back() == &it->second;
    }

    bool isActive(const char* name) {
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
