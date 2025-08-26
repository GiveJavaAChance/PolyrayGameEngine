package polyray.systems.entity;

import java.util.ArrayList;

public abstract class Script implements Component {

    Entity parent;

    public abstract void setup();

    public abstract void frameUpdate(double dt);

    public abstract void physicsUpdate(double dt);

    protected final <T extends Component> T getComponent(Class<T> type) {
        return parent.getComponent(type);
    }

    protected final <T extends Component> ArrayList<T> getComponents(Class<T> type) {
        ArrayList<T> components = new ArrayList<>();
        parent.getComponents(type, components);
        return components;
    }
}
