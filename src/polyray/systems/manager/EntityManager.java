package polyray.systems.manager;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import polyray.builtin.RenderObject;
import polyray.modular.RenderObjectBase;
import polyray.systems.entity.Controller;
import polyray.systems.entity.Entity;
import polyray.systems.registry.EntityRenderRegistry;

public class EntityManager {

    private static double tickAccu, tickH;
    private static double physAccu, physH = 0.006d;
    private static double timeoutThreshold;

    private static final ArrayList<Entity> entities = new ArrayList<>();
    private static final HashMap<Entity, ArrayList<Controller>> controllerMap = new HashMap<>();

    public static final void setTickSpeed(double time) {
        tickH = time;
    }

    public static final void setPhysicsSpeed(double time) {
        physH = time;
    }

    public static final void setTimeoutThreshold(double thresh) {
        timeoutThreshold = thresh;
    }

    private static void updateTicks() {
        Iterator<Entity> iter = entities.iterator();
        while (iter.hasNext()) {
            Entity e = iter.next();
            if (e.dead) {
                if (e.deathTime <= 0) {
                    if (e.instance != null) {
                        EntityRenderRegistry.getRenderObject(e.getClass()).removeInstance(e.instance);
                    }
                    controllerMap.remove(e);
                    iter.remove();
                } else {
                    e.deathTime--;
                }
            }
        }
        ParticleManager.updateTicks();
    }

    public static final void update(double dt) {
        physAccu += dt;
        long startTime = System.nanoTime();
        while (physAccu >= physH) {
            for (Entity e : entities) {
                ArrayList<Controller> c = controllerMap.get(e);
                if (c != null) {
                    for (Controller cont : c) {
                        cont.update(e, physH);
                    }
                }
                e.update(physH);
            }
            ParticleManager.update(physH);
            physAccu -= physH;
            if ((System.nanoTime() - startTime) / 1000000000.0d > timeoutThreshold) {
                physAccu = 0.0d;
                break;
            }
        }
        for (Entity e : entities) {
            if (e.instance != null) {
                e.updateInstance();
            }
        }
        for (RenderObjectBase obj : EntityRenderRegistry.getRenderObjects()) {
            obj.uploadInstances();
        }
        ParticleManager.updateInstances();
        startTime = System.nanoTime();
        tickAccu += dt;
        while (tickAccu >= tickH) {
            tickAccu -= tickH;
            updateTicks();
            if ((System.nanoTime() - startTime) / 1000000000.0d > timeoutThreshold) {
                tickAccu = 0.0d;
                break;
            }
        }
    }

    public static final void spawn(Entity e) {
        entities.add(e);
        e.instance = EntityRenderRegistry.createInstance(e.getClass());
        EntityRenderRegistry.getRenderObject(e.getClass()).addInstance(e.instance);
    }

    public static final void spawnExcluded(Entity e) {
        entities.add(e);
    }

    public static final void registerController(Entity e, Controller c) {
        controllerMap.computeIfAbsent(e, k -> new ArrayList<>()).add(c);
    }

    public static final void releaseController(Entity e, Controller c) {
        ArrayList<Controller> controllers = controllerMap.get(e);
        if(controllers == null) {
            return;
        }
        controllers.remove(c);
        if(controllers.isEmpty()) {
            controllerMap.remove(e);
        }
    }

}
