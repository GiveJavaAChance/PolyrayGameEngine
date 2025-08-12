package polyray.systems.registry;

import java.util.Collection;
import java.util.HashMap;
import java.util.function.Supplier;
import polyray.builtin.RenderObject;
import polyray.modular.Instance;
import polyray.modular.Vertex;

public class RenderRegistry<T> {

    private final HashMap<Class<? extends T>, Integer> classReg = new HashMap<>();
    private final HashMap<Class<? extends T>, RenderObject<?, Instance>> renderObjects = new HashMap<>();
    private final HashMap<Class<? extends T>, Supplier<? extends Instance>> instanceSuppliers = new HashMap<>();

    public final <U extends Instance> void registerRender(Class<? extends T> clazz, RenderObject<?, Instance> obj, Supplier<U> instanceSupplier) {
        int id = classReg.size();
        classReg.put(clazz, id);
        renderObjects.put(clazz, obj);
        instanceSuppliers.put(clazz, instanceSupplier);
    }

    public final int getID(Class<? extends T> clazz) {
        return classReg.get(clazz);
    }

    public final RenderObject<?, Instance> getRenderObject(Class<? extends T> clazz) {
        return renderObjects.get(clazz);
    }

    public final void addInstance(Class<? extends T> clazz, Instance i) {
        RenderObject<?, Instance> obj = renderObjects.get(clazz);
        if (obj != null) {
            obj.addInstance(i);
        }
    }

    public final void removeInstance(Class<? extends T> clazz, Instance i) {
        RenderObject<?, Instance> obj = renderObjects.get(clazz);
        if (obj != null) {
            obj.removeInstance(i);
        }
    }

    public final Instance createInstance(Class<? extends T> clazz) {
        Supplier<? extends Instance> supplier = instanceSuppliers.get(clazz);
        return supplier != null ? supplier.get() : null;
    }

    public final Collection<RenderObject<?, Instance>> getRenderObjects() {
        return renderObjects.values();
    }
}
