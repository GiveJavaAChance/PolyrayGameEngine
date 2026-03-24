#ifndef EVENTBUS_H_INCLUDED
#define EVENTBUS_H_INCLUDED

#pragma once

#include <cstdint>
#include <memory>
#include "DynamicArray.h"
#include "typereg.h"

using EventListener = bool(*)(void* const evt);

struct EventBus {
private:
    DynamicArray<DynamicArray<EventListener>> listeners;

    template<typename Event>
    inline void fireImpl(Event& evt) {
        uint32_t ID = EventTypes::getTypeId<Event>();
        DynamicArray<EventListener>& l = listeners[ID];
        void* ptr = &evt;
        for(uint32_t i = 0u; i < l.size(); i++) {
            if(l[i](ptr)) {
                break;
            }
        }
    }

    TYPE_REGISTRY(EventTypes)
public:
    template<typename Event, bool(*onEvent)(Event* const evt)>
    void registerEventListener() {
        uint32_t ID = EventTypes::getTypeId<Event>();
        while(ID >= listeners.size()) {
            listeners.emplace(1u);
        }
        listeners[ID].add(reinterpret_cast<EventListener>(onEvent));
    }

    template<typename Event>
    void fire(Event& evt) {
        fireImpl(evt);
    }

    template<typename Event, typename... Args>
    void fireDirect(Args&&... args) {
        alignas(Event) unsigned char storage[sizeof(Event)];
        Event* evt = std::construct_at(reinterpret_cast<Event*>(storage), std::forward<Args>(args)...);
        fireImpl(*evt);
        evt->~Event();
    }
};

#endif
