package polyray.systems.registry;

import java.util.Collection;
import java.util.HashMap;
import java.util.function.Supplier;
import polyray.modular.Instance;
import polyray.modular.RenderObjectBase;
import polyray.systems.ParticleObj;

public class ParticleRenderRegistry {

    private static final HashMap<Class<? extends ParticleObj>, Integer> classReg = new HashMap<>();
    private static final HashMap<Class<? extends ParticleObj>, RenderObjectBase> renderObjects = new HashMap<>();
    private static final HashMap<Class<? extends ParticleObj>, Supplier<? extends Instance>> instanceSuppliers = new HashMap<>();

    public static final <T extends Instance> void registerParticleRender(Class<? extends ParticleObj> clazz, RenderObjectBase obj, Supplier<T> instanceSupplier) {
        int id = classReg.size();
        classReg.put(clazz, id);
        renderObjects.put(clazz, obj);
        instanceSuppliers.put(clazz, instanceSupplier);
    }

    public static final int getID(Class<? extends ParticleObj> clazz) {
        return classReg.get(clazz);
    }

    public static final RenderObjectBase getRenderObject(Class<? extends ParticleObj> clazz) {
        return renderObjects.get(clazz);
    }

    public static final Instance createInstance(Class<? extends ParticleObj> clazz) {
        Supplier<? extends Instance> supplier = instanceSuppliers.get(clazz);
        return supplier != null ? supplier.get() : null;
    }
    
    public static final Collection<RenderObjectBase> getRenderObjects() {
        return renderObjects.values();
    }
}