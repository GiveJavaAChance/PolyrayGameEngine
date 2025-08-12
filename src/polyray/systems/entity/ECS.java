package polyray.systems.entity;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.function.Consumer;
import polyray.physics.PhysicsObject;
import polyray.systems.BVH;
import polyray.systems.manager.PhysicsManager3D;
import polyray.physics.Collider3D;

public class ECS {

    private static final HashMap<Class<? extends Component>, Consumer<? extends Component>> systems = new HashMap<>();
    private static final HashMap<Class<? extends Component>, Consumer<? extends Component>> systemCache = new HashMap<>();

    static {
        registerSystem(EntityCollider3D.class, PhysicsManager3D::addCollider);
    }

    public static final ArrayList<Entity> entities = new ArrayList<>();

    public static <T extends Component> void registerSystem(Class<T> clazz, Consumer<T> add) {
        systems.put(clazz, add);
    }

    @SuppressWarnings("unchecked")
    public static void spawn(Entity e) {
        for (Component c : e.components) {
            Class<? extends Component> clazz = c.getClass();

            Consumer<? extends Component> add = systemCache.get(clazz);
            if (add == null) {
                for (Entry<Class<? extends Component>, Consumer<? extends Component>> entry : systems.entrySet()) {
                    if (entry.getKey().isAssignableFrom(clazz)) {
                        add = entry.getValue();
                        break;
                    }
                }
                systemCache.put(clazz, add);
            }

            if (add != null) {
                ((Consumer<Component>) add).accept(c);
            }
        }
    }
}
