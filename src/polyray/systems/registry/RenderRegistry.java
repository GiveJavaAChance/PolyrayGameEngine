package polyray.systems.registry;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.function.Supplier;
import polyray.builtin.RenderObject;
import polyray.modular.Instance;

public class RenderRegistry<T> {

    private final HashMap<Class<? extends T>, Integer> classReg = new HashMap<>();
    private final HashMap<Class<? extends T>, RenderObject> renderObjects = new HashMap<>();
    private final HashMap<Class<? extends T>, ArrayList<Instance>> renderObjectInstances = new HashMap<>();
    private final HashMap<Class<? extends T>, Supplier<? extends Instance>> instanceSuppliers = new HashMap<>();

    public final <U extends Instance> void registerRender(Class<? extends T> clazz, RenderObject obj, ArrayList<Instance> instances, Supplier<U> instanceSupplier) {
        int id = classReg.size();
        classReg.put(clazz, id);
        renderObjects.put(clazz, obj);
        renderObjectInstances.put(clazz, instances);
        instanceSuppliers.put(clazz, instanceSupplier);
    }

    public final int getID(Class<? extends T> clazz) {
        return classReg.get(clazz);
    }

    public final RenderObject getRenderObject(Class<? extends T> clazz) {
        return renderObjects.get(clazz);
    }

    public final void addInstance(Class<? extends T> clazz, Instance i) {
        ArrayList<Instance> instances = renderObjectInstances.get(clazz);
        if (instances != null) {
            instances.add(i);
        }
    }

    public final void removeInstance(Class<? extends T> clazz, Instance i) {
        ArrayList<Instance> instances = renderObjectInstances.get(clazz);
        if (instances != null) {
            instances.remove(i);
        }
    }

    public final Instance createInstance(Class<? extends T> clazz) {
        Supplier<? extends Instance> supplier = instanceSuppliers.get(clazz);
        return supplier != null ? supplier.get() : null;
    }

    public final Collection<RenderObject> getRenderObjects() {
        return renderObjects.values();
    }
}
