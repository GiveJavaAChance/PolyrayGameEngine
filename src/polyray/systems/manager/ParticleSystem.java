package polyray.systems.manager;

import java.util.ArrayList;
import java.util.Iterator;
import polyray.builtin.RenderObject;
import polyray.systems.Particle;
import polyray.systems.registry.ParticleRenderRegistry;

public class ParticleSystem {

    private static final ArrayList<Particle> particles = new ArrayList<>();

    public static final void tickUpdate(double dt) {
        Iterator<Particle> iter = particles.iterator();
        while (iter.hasNext()) {
            Particle p = iter.next();
            if (p.time <= 0) {
                if (p.instance != null) {
                    ParticleRenderRegistry.removeInstance(p.getClass(), p.instance);
                }
                iter.remove();
            } else {
                p.tickUpdate();
                p.time--;
            }
        }
    }

    public static final void physicsUpdate(double dt) {
        for (Particle p : particles) {
            p.physicsUpdate(dt);
        }
    }

    public static final void frameUpdate() {
        for (Particle p : particles) {
            if (p.instance != null) {
                p.updateInstance();
            }
        }
        for (RenderObject obj : ParticleRenderRegistry.getRenderObjects()) {
            obj.uploadInstances();
        }
    }

    @SuppressWarnings("unchecked")
    public static final void spawn(Particle p) {
        particles.add(p);
        p.instance = ParticleRenderRegistry.createInstance(p.getClass());
        ParticleRenderRegistry.addInstance(p.getClass(), p.instance);
    }
}
