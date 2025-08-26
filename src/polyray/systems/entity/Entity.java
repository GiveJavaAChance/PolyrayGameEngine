package polyray.systems.entity;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

public class Entity {

    protected final ArrayList<Object> components;

    public Entity() {
        this.components = new ArrayList<>();
    }

    public Entity(Object... components) {
        this();
        this.components.addAll(Arrays.asList(components));
    }

    public final void addComponent(Object c) {
        this.components.add(c);
    }

    public final void removeComponent(Object c) {
        this.components.remove(c);
    }

    @SuppressWarnings("unchecked")
    public final <T> void getComponents(Class<T> clazz, Collection<T> components) {
        for (Object c : this.components) {
            if (clazz.isAssignableFrom(c.getClass())) {
                components.add((T) c);
            }
        }
    }

    @SuppressWarnings("unchecked")
    public final <T> T getComponent(Class<T> clazz) {
        for (Object c : components) {
            if (clazz.isAssignableFrom(c.getClass())) {
                return (T) c;
            }
        }
        return null;
    }
}
