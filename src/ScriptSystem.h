#ifndef SCRIPTSYSTEM_H_INCLUDED
#define SCRIPTSYSTEM_H_INCLUDED

#pragma once

#include <World.h>
#include <input/InputEvent.h>
#include <serial/Serial.h>
#include <typereg.h>

#define SCRIPT                                                          \
    uint32_t entityID;                                                  \
    World* world;                                                       \
    template <typename T>                                               \
    inline void addComponent(T&& component) {                           \
        world->ecs.addComponent(entityID, std::move(component));        \
    }                                                                   \
    template <typename T>                                               \
    inline bool setComponent(T&& component) {                           \
        return world->ecs.setComponent(entityID, std::move(component)); \
    }                                                                   \
    template <typename T>                                               \
    inline void removeComponent() {                                     \
        world->ecs.removeComponent<T>(entityID);                        \
    }                                                                   \
    template <typename T>                                               \
    inline bool getComponent(T& out) {                                  \
        return world->ecs.getComponent(entityID, out);                  \
    }                                                                   \
    template <typename T>                                               \
    inline T* getComponentPtr() {                                       \
        return world->ecs.getComponentPtr<T>(entityID);                 \
    }                                                                   \
    template <typename T>                                               \
    inline uint32_t getComponentCount() {                               \
        return world->ecs.getComponentCount<T>(entityID);               \
    }                                                                   \
    template <typename T>                                               \
    inline uint32_t getComponents(T* const ptr) {                       \
        return world->ecs.getComponents(entityID, ptr);                 \
    }

#define REGISTER_SCRIPT(name)                                     \
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
    { t.entityID } -> std::convertible_to<uint32_t&>;
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
    static_assert(IsScript<T>, "Type given is not a script.");

private:
    World* world;

    void onComponentAdded(Entity e, uint32_t id) {
        T* s = world->ecs.getPtr<T>(id);
        s->entityID = e.entityID;
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
        ecs.registerComponentListener<T, ScriptSystem<T>, &ScriptSystem<T>::onComponentAdded, &ScriptSystem<T>::onComponentRemoved>(this);
        if constexpr (HasSetup<T>) {
            ecs.registerSetupCallback<ScriptSystem<T>, &ScriptSystem<T>::setup>(this);
        }
        if constexpr (HasFrameUpdate<T>) {
            ecs.registerUpdateCallback<ScriptSystem<T>, &ScriptSystem<T>::frameUpdate, UpdateOrder::FRAME>(this);
        }
        if constexpr (HasPhysicsUpdate<T>) {
            ecs.registerUpdateCallback<ScriptSystem<T>, &ScriptSystem<T>::physicsUpdate, UpdateOrder::PHYSICS>(this);
        }
        if constexpr (HasInput<T>) {
            if (!disabled) {
                world->eventBus.registerEventListener<InputEvent, ScriptSystem<T>, &ScriptSystem<T>::input>(this);
            }
        }
    }
};

#endif
