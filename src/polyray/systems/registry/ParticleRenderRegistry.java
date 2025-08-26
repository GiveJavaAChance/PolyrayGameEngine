package polyray.systems.registry;

import java.util.ArrayList;
import java.util.Collection;
import java.util.function.Supplier;
import polyray.builtin.RenderObject;
import polyray.modular.Instance;
import polyray.systems.Particle;

public class ParticleRenderRegistry {

    private static final RenderRegistry<Particle> reg = new RenderRegistry<>();

    public static final <T extends Instance> void registerParticleRender(Class<? extends Particle> clazz, RenderObject<?, Instance> obj, Supplier<T> instanceSupplier) {
        reg.registerRender(clazz, obj, instanceSupplier);
    }

    public static final int getID(Class<? extends Particle> clazz) {
        return reg.getID(clazz);
    }

    public static final RenderObject<?, Instance> getRenderObject(Class<? extends Particle> clazz) {
        return reg.getRenderObject(clazz);
    }

    public static final Instance createInstance(Class<? extends Particle> clazz) {
        return reg.createInstance(clazz);
    }

    public static final void addInstance(Class<? extends Particle> clazz, Instance i) {
        reg.addInstance(clazz, i);
    }

    public static final void removeInstance(Class<? extends Particle> clazz, Instance i) {
        reg.removeInstance(clazz, i);
    }

    public static final Collection<RenderObject<?, Instance>> getRenderObjects() {
        return reg.getRenderObjects();
    }
}
