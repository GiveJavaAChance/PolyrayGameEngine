package polyray.systems.entity;

import java.util.ArrayList;
import java.util.Collection;

public class Entity {

    protected final ArrayList<Component> components = new ArrayList<>();
    
    public final void addComponent(Component c) {
        this.components.add(c);
    }
    
    public final void removeComponent(Component c) {
        this.components.remove(c);
    }
    
    @SuppressWarnings("unchecked")
    public final <T extends Component> void getComponents(Class<T> clazz, Collection<T> components) {
        for(Component c : this.components) {
            if(clazz.isAssignableFrom(c.getClass())) {
                components.add((T) c);
            }
        }
    }
    
    @SuppressWarnings("unchecked")
    public final <T extends Component> T getComponent(Class<T> clazz) {
        for(Component c : components) {
            if(clazz.isAssignableFrom(c.getClass())) {
                return (T) c;
            }
        }
        return null;
    }
}
