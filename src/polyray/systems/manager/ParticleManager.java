package polyray.systems.manager;

import java.util.ArrayList;
import java.util.Iterator;
import polyray.modular.RenderObjectBase;
import polyray.systems.ParticleObj;
import polyray.systems.registry.ParticleRenderRegistry;

public class ParticleManager {

    private static final ArrayList<ParticleObj> particles = new ArrayList<>();

    protected static final void updateTicks() {
        Iterator<ParticleObj> iter = particles.iterator();
        while (iter.hasNext()) {
            ParticleObj p = iter.next();
            if (p.time <= 0) {
                if (p.instance != null) {
                    ParticleRenderRegistry.getRenderObject(p.getClass()).removeInstance(p.instance);
                }
                iter.remove();
            } else {
                p.tick();
                p.time--;
            }
        }
    }

    protected static final void update(double dt) {
        for (ParticleObj p : particles) {
            p.update(dt);
        }
    }

    protected static final void updateInstances() {
        for (ParticleObj p : particles) {
            if (p.instance != null) {
                p.updateInstance();
            }
        }
        for (RenderObjectBase obj : ParticleRenderRegistry.getRenderObjects()) {
            obj.uploadInstances();
        }
    }
    
    public static final void spawn(ParticleObj p) {
        particles.add(p);
        p.instance = ParticleRenderRegistry.createInstance(p.getClass());
        ParticleRenderRegistry.getRenderObject(p.getClass()).addInstance(p.instance);
    }
}
