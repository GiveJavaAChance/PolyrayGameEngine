package polyray.systems.registry;

import java.util.Collection;
import java.util.HashMap;
import java.util.function.Supplier;
import polyray.modular.Instance;
import polyray.modular.RenderObjectBase;
import polyray.systems.entity.Entity;

public class EntityRenderRegistry {

    private static final HashMap<Class<? extends Entity>, Integer> classReg = new HashMap<>();
    private static final HashMap<Class<? extends Entity>, RenderObjectBase> renderObjects = new HashMap<>();
    private static final HashMap<Class<? extends Entity>, Supplier<? extends Instance>> instanceSuppliers = new HashMap<>();

    public static final <T extends Instance> void registerEntityRender(Class<? extends Entity> clazz, RenderObjectBase obj, Supplier<T> instanceSupplier) {
        int id = classReg.size();
        classReg.put(clazz, id);
        renderObjects.put(clazz, obj);
        instanceSuppliers.put(clazz, instanceSupplier);
    }

    public static final int getID(Class<? extends Entity> clazz) {
        return classReg.get(clazz);
    }

    public static final RenderObjectBase getRenderObject(Class<? extends Entity> clazz) {
        return renderObjects.get(clazz);
    }

    public static final Instance createInstance(Class<? extends Entity> clazz) {
        Supplier<? extends Instance> supplier = instanceSuppliers.get(clazz);
        return supplier != null ? supplier.get() : null;
    }
    
    public static final Collection<RenderObjectBase> getRenderObjects() {
        return renderObjects.values();
    }
}