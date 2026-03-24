#ifndef SCRIPTSYSTEM_H_INCLUDED
#define SCRIPTSYSTEM_H_INCLUDED

#pragma once

#include "ECS.h"
#include "typereg.h"

struct Script {
private:
    Entity e;

protected:
    template<Component T>
    inline void addComponent(const T& component) {
        e.addComponent(component);
    }

    template<Component T>
    inline bool setComponent(const T& component) {
        return e.setComponent(component);
    }

    template<Component T>
    inline void removeComponent() {
        e.removeComponent<T>();
    }

    template<Component T>
    inline bool getComponent(T& out) {
        return e.getComponent(out);
    }

    template<Component T>
    inline T* getComponentPtr() {
        return e.getComponentPtr<T>();
    }

    template<Component T>
    inline uint32_t getComponentCount() {
        return e.getComponentCount<T>();
    }

    template<Component T>
    inline uint32_t getComponents(T* const ptr) {
        return e.getComponents(ptr);
    }

public:
    Script() : e(0u, nullptr) {
    }

    inline void setEntity(const Entity& e) noexcept {
        this->e = e;
    }
};

template<typename T>
concept HasSetup = requires(T& t) {
    t.setup();
};

template<typename T>
concept HasFrameUpdate = requires(T& t, double dt) {
    t.frameUpdate(dt);
};

template<typename T>
concept HasPhysicsUpdate = requires(T& t, double dt) {
    t.physicsUpdate(dt);
};

template<typename T>
struct ScriptSystem {
private:
    ECS* ecs;

    void onComponentAdded(Entity e, uint32_t id) {
        T& s = ecs->view<T>().get(id);
        s.setEntity(e);
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    void setup() {
        Storage<T>& storage = ecs->view<T>();
        for(uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].setup();
        }
    }

    void frameUpdate(double dt) {
        Storage<T>& storage = ecs->view<T>();
        for (uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].frameUpdate(dt);
        }
    }

    void physicsUpdate(double dt) {
        Storage<T>& storage = ecs->view<T>();
        for (uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].physicsUpdate(dt);
        }
    }

public:
    ScriptSystem(ECS* ecs) : ecs(ecs) {
        ecs->registerComponentType<T>();
        ecs->registerComponentListener<T, ScriptSystem<T>, onComponentAdded, onComponentRemoved>(this);
        if constexpr (HasSetup<T>) {
            ecs->registerSetupCallback<ScriptSystem<T>, setup>(this);
        }
        if constexpr (HasFrameUpdate<T>) {
            ecs->registerUpdateCallback<ScriptSystem<T>, frameUpdate, UpdateOrder::FRAME>(this);
        }
        if constexpr (HasPhysicsUpdate<T>) {
            ecs->registerUpdateCallback<ScriptSystem<T>, physicsUpdate, UpdateOrder::PHYSICS>(this);
        }
    }
};

#endif
