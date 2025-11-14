package polyray.systems.entity;

@FunctionalInterface
public interface EventListener<E extends GameEvent> {

    void onEvent(E e, EventBus bus);
}
