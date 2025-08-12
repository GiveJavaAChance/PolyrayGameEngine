package polyray.systems.registry;

import java.util.ArrayList;
import java.util.Collection;
import java.util.function.Supplier;
import polyray.builtin.RenderObject;
import polyray.modular.Instance;
import polyray.systems.ParticleObj;

public class ParticleRenderRegistry {

    private static final RenderRegistry<ParticleObj> reg = new RenderRegistry<>();

    public static final <T extends Instance> void registerParticleRender(Class<? extends ParticleObj> clazz, RenderObject<?, Instance> obj, Supplier<T> instanceSupplier) {
        reg.registerRender(clazz, obj, instanceSupplier);
    }

    public static final int getID(Class<? extends ParticleObj> clazz) {
        return reg.getID(clazz);
    }

    public static final RenderObject<?, Instance> getRenderObject(Class<? extends ParticleObj> clazz) {
        return reg.getRenderObject(clazz);
    }

    public static final Instance createInstance(Class<? extends ParticleObj> clazz) {
        return reg.createInstance(clazz);
    }

    public static final void addInstance(Class<? extends ParticleObj> clazz, Instance i) {
        reg.addInstance(clazz, i);
    }

    public static final void removeInstance(Class<? extends ParticleObj> clazz, Instance i) {
        reg.removeInstance(clazz, i);
    }

    public static final Collection<RenderObject<?, Instance>> getRenderObjects() {
        return reg.getRenderObjects();
    }
}
