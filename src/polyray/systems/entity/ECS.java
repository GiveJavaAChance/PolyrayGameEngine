package polyray.systems.entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.function.Consumer;
import java.util.function.DoubleConsumer;
import polyray.systems.IDGenerator;

public class ECS {

    private static final ArrayList<Update> updates = new ArrayList<>();

    private static double fixedDt = 0.006d;
    private static double timeoutThreshold = 0.003d;
    private static double timeRemaining = 0.0d;

    private static final ArrayList<ECSSystem> systems = new ArrayList<>();

    private static final IDGenerator gen = new IDGenerator();
    private static final HashMap<Integer, Entity> entities = new HashMap<>();

    public static void register2DPhysics() {
        registerSystem(null, Physics2DSystem::addEntity, Physics2DSystem::removeEntity);
        registerPhysicsUpdate(Physics2DSystem::physicsUpdate);
    }

    public static void register3DPhysics() {
        registerSystem(null, Physics3DSystem::addEntity, Physics3DSystem::removeEntity);
        registerPhysicsUpdate(Physics3DSystem::physicsUpdate);
    }

    public static void registerScripting() {
        registerSystem(null, ScriptingSystem::addEntity, ScriptingSystem::removeEntity);
        registerFrameUpdate(ScriptingSystem::frameUpdate);
        registerPhysicsUpdate(ScriptingSystem::physicsUpdate);
    }

    public static void registerSystem(Runnable setup, Consumer<Entity> add, Consumer<Entity> remove) {
        systems.add(new ECSSystem(setup, add, remove));
    }

    public static void registerFrameUpdate(DoubleConsumer frameUpdate) {
        updates.add(new Update(frameUpdate, false));
    }

    public static void registerPhysicsUpdate(DoubleConsumer physicsUpdate) {
        updates.add(new Update(physicsUpdate, true));
    }

    public static void setup() {
        for (ECSSystem system : systems) {
            system.setup();
        }
    }

    public static void update(double dt) {
        timeRemaining += dt;
        int substeps = 0;
        while (timeRemaining >= fixedDt) {
            timeRemaining -= fixedDt;
            substeps++;
        }
        for (Update update : updates) {
            if (update.type) {
                long startTime = System.nanoTime();
                for (int i = 0; i < substeps; i++) {
                    update.update.accept(fixedDt);
                    if ((System.nanoTime() - startTime) / 1000000000.0d > timeoutThreshold) {
                        break;
                    }
                }
            } else {
                update.update.accept(dt);
            }
        }
    }

    public static int spawn(Entity e) {
        int ID = gen.getNewID();
        entities.put(ID, e);
        for (ECSSystem system : systems) {
            system.add(e);
        }
        return ID;
    }

    public static boolean kill(int ID) {
        Entity e = entities.remove(ID);
        if (e == null) {
            return false;
        }
        for (ECSSystem system : systems) {
            system.remove(e);
        }
        return true;
    }

    public static Entity getEntityByID(int ID) {
        return entities.get(ID);
    }

    private static class ECSSystem {

        public final Runnable setup;
        public final Consumer<Entity> add;
        public final Consumer<Entity> remove;

        public ECSSystem(Runnable setup, Consumer<Entity> add, Consumer<Entity> remove) {
            this.setup = setup;
            this.add = add;
            this.remove = remove;
        }

        public final void setup() {
            if (setup != null) {
                setup.run();
            }
        }

        public final void add(Entity e) {
            add.accept(e);
        }

        public final void remove(Entity e) {
            remove.accept(e);
        }
    }

    private static class Update {

        public final DoubleConsumer update;
        public final boolean type;

        public Update(DoubleConsumer update, boolean type) {
            this.update = update;
            this.type = type;
        }
    }
}
