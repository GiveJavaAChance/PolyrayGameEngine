#ifndef EVENTBUS_H_INCLUDED
#define EVENTBUS_H_INCLUDED

#pragma once

#include <InvokeUtils.h>
#include <cstdint>
#include <memory>
#include <structure/DynamicArray.h>
#include <typereg.h>

struct EventBus {
private:
    struct EventListener {
        void* instance;
        bool (*func)(void*, void*);
    };

    DynamicArray<DynamicArray<EventListener>> listeners;

    template <typename Event>
    inline void fireImpl(Event* evt) {
        uint32_t ID = EventTypes::getTypeId<Event>();
        DynamicArray<EventListener>& l = listeners[ID];
        for (uint32_t i = 0u; i < l.size(); i++) {
            EventListener& listener = l[i];
            if (listener.func(listener.instance, evt)) {
                break;
            }
        }
    }

    TYPE_REGISTRY(EventTypes)
public:
    template <typename Event, bool (*OnEvent)(Event*)>
    void registerEventListener() {
        uint32_t ID = EventTypes::getTypeId<Event>();
        while (ID >= listeners.size()) {
            listeners.emplace(1u);
        }
        listeners[ID].emplace(nullptr, reinterpret_cast<bool (*)(void*, void*)>(&Invoke<Event*>::template wrapThunkReturn<bool, OnEvent>));
    }

    template <typename Event, typename T, bool (T::*OnEvent)(Event* const)>
    void registerEventListener(T* instance) {
        uint32_t ID = EventTypes::getTypeId<Event>();
        while (ID >= listeners.size()) {
            listeners.emplace(1u);
        }
        listeners[ID].emplace(instance, reinterpret_cast<bool (*)(void*, void*)>(&Invoke<Event*>::template thunkReturn<T, bool, OnEvent>));
    }

    template <typename Event>
    void fire(Event& evt) {
        fireImpl(&evt);
    }

    template <typename Event, typename... Args>
    void fireDirect(Args&&... args) {
        alignas(Event) unsigned char storage[sizeof(Event)];
        Event* evt = std::construct_at(reinterpret_cast<Event*>(storage), std::forward<Args>(args)...);
        fireImpl(evt);
        evt->~Event();
    }
};

#endif
