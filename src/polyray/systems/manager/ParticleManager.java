package polyray.systems.manager;

import java.util.ArrayList;
import java.util.Iterator;
import polyray.builtin.RenderObject;
import polyray.systems.ParticleObj;
import polyray.systems.registry.ParticleRenderRegistry;

public class ParticleManager {

    private static final ArrayList<ParticleObj> particles = new ArrayList<>();

    public static final void updateTicks(double dt) {
        Iterator<ParticleObj> iter = particles.iterator();
        while (iter.hasNext()) {
            ParticleObj p = iter.next();
            if (p.time <= 0) {
                if (p.instance != null) {
                    ParticleRenderRegistry.removeInstance(p.getClass(), p.instance);
                }
                iter.remove();
            } else {
                p.tick();
                p.time--;
            }
        }
    }

    public static final void update(double dt) {
        for (ParticleObj p : particles) {
            p.update(dt);
        }
    }

    public static final void updateInstances() {
        for (ParticleObj p : particles) {
            if (p.instance != null) {
                p.updateInstance();
            }
        }
        for (RenderObject obj : ParticleRenderRegistry.getRenderObjects()) {
            obj.uploadInstances();
        }
    }

    public static final void spawn(ParticleObj p) {
        particles.add(p);
        p.instance = ParticleRenderRegistry.createInstance(p.getClass());
        ParticleRenderRegistry.addInstance(p.getClass(), p.instance);
    }
}
