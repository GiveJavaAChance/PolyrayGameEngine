#ifndef SCRIPTSYSTEM_H_INCLUDED
#define SCRIPTSYSTEM_H_INCLUDED

#pragma once

#include <World.h>
#include <input/InputEvent.h>
#include <serial/Serial.h>
#include <typereg.h>

#define SCRIPT                                 \
Entity entity;                                 \
World* world;                                  \
template <Component T>                         \
inline void addComponent(const T& component) { \
    entity.addComponent(component);            \
}                                              \
template <Component T>                         \
inline bool setComponent(const T& component) { \
    return entity.setComponent(component);     \
}                                              \
template <Component T>                         \
inline void removeComponent() {                \
    entity.removeComponent<T>();               \
}                                              \
template <Component T>                         \
inline bool getComponent(T& out) {             \
    return entity.getComponent(out);           \
}                                              \
template <Component T>                         \
inline T* getComponentPtr() {                  \
    return entity.getComponentPtr<T>();        \
}                                              \
template <Component T>                         \
inline uint32_t getComponentCount() {          \
    return entity.getComponentCount<T>();      \
}                                              \
template <Component T>                         \
inline uint32_t getComponents(T* const ptr) {  \
    return entity.getComponents(ptr);          \
}

#define REGISTER_SCRIPT(name)                                 \
template <>                                                   \
struct Serial<name> {                                         \
    static void serialize(World*, uint32_t, ByteWriter&) {    \
    }                                                         \
    static void deserialize(World*, Entity& e, ByteReader&) { \
        e.addComponent(name{});                               \
    }                                                         \
};

template <typename T>
concept IsScript = requires(T t) {
    { t.entity } -> std::convertible_to<Entity&>;
    { t.world } -> std::convertible_to<World*>;
};

template <typename T>
concept HasSetup = requires(T& t) {
    t.setup();
};

template <typename T>
concept HasFrameUpdate = requires(T& t, double dt) {
    t.frameUpdate(dt);
};

template <typename T>
concept HasPhysicsUpdate = requires(T& t, double dt) {
    t.physicsUpdate(dt);
};

template <typename T>
concept HasInput = requires(T& t, const InputEvent& event) {
    t.input(event);
};

template <typename T>
struct ScriptSystem {
    static_assert(std::is_standard_layout_v<T>, "Script types must be [StandardLayoutType](https://en.cppreference.com/cpp/language/classes#Standard-layout_class).");
    static_assert(IsScript<T>, "Type given is not a script.");

private:
    World* world;

    void onComponentAdded(Entity e, uint32_t id) {
        T* s = world->ecs.getPtr<T>(id);
        s->entity = e;
        s->world = world;
    }

    void onComponentRemoved(Entity e, uint32_t id) {
    }

    void setup() {
        Storage<T>& storage = world->ecs.view<T>();
        for (uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].setup();
        }
    }

    void frameUpdate(double dt) {
        Storage<T>& storage = world->ecs.view<T>();
        for (uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].frameUpdate(dt);
        }
    }

    void physicsUpdate(double dt) {
        Storage<T>& storage = world->ecs.view<T>();
        for (uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].physicsUpdate(dt);
        }
    }

    bool input(InputEvent* event) {
        Storage<T>& storage = world->ecs.view<T>();
        for (uint32_t i = 0u; i < storage.data.size(); i++) {
            storage.data[i].input(*event);
        }
        return false;
    }

public:
    ScriptSystem(World* world, bool disabled) : world(world) {
        ECS& ecs = world->ecs;
        ecs.registerComponentListener<T, ScriptSystem<T>, onComponentAdded, onComponentRemoved>(this);
        if constexpr (HasSetup<T>) {
            ecs.registerSetupCallback<ScriptSystem<T>, setup>(this);
        }
        if constexpr (HasFrameUpdate<T>) {
            ecs.registerUpdateCallback<ScriptSystem<T>, frameUpdate, UpdateOrder::FRAME>(this);
        }
        if constexpr (HasPhysicsUpdate<T>) {
            ecs.registerUpdateCallback<ScriptSystem<T>, physicsUpdate, UpdateOrder::PHYSICS>(this);
        }
        if constexpr (HasInput<T>) {
            if (!disabled) {
                world->eventBus.registerEventListener<InputEvent, ScriptSystem<T>, input>(this);
            }
        }
    }
};

#endif
