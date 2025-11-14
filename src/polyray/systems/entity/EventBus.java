package polyray.systems.entity;

import java.util.ArrayList;
import java.util.HashMap;

public class EventBus {

    private final HashMap<Class<?>, ArrayList<EventListener<?>>> listeners;

    public EventBus() {
        this.listeners = new HashMap<>();
    }

    public <E extends GameEvent> void register(Class<E> clazz, EventListener<E> l) {
        listeners.computeIfAbsent(clazz, k -> new ArrayList<>()).add(l);
    }

    @SuppressWarnings("unchecked")
    public <E extends GameEvent> void fire(E e) {
        ArrayList<EventListener<?>> ll = listeners.get(e.getClass());
        if (ll == null) {
            return;
        }
        for (EventListener l : ll) {
            try {
                l.onEvent(e, this);
            } catch (Exception ex) {
            }
            if (e.cancelled) {
                break;
            }
        }
    }
}
