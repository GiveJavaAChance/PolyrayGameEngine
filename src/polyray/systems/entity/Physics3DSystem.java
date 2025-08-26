package polyray.systems.entity;

import java.util.ArrayList;
import polyray.physics.PhysicsObject;
import polyray.systems.BVH;
import polyray.physics.Collider3D;

public class Physics3DSystem {

    private static final ArrayList<PhysicsObject> objects = new ArrayList<>();
    private static final ArrayList<Collider3D> colliders = new ArrayList<>();

    public static final void addEntity(Entity e) {
    }

    public static final void removeEntity(Entity e) {
    }

    public static final void addObject(PhysicsObject object) {
        objects.add(object);
    }

    public static final void removeObject(PhysicsObject object) {
        objects.remove(object);
    }

    public static final void addCollider(Collider3D collider) {
        colliders.add(collider);
    }

    public static final void removeCollider(Collider3D collider) {
        colliders.remove(collider);
    }

    public static final void physicsUpdate(double dt) {
        for (PhysicsObject obj : objects) {
            obj.update(dt);
        }
        float[][] bounds = new float[colliders.size()][0];
        for (int i = 0; i < colliders.size(); i++) {
            float[] box = new float[6];
            colliders.get(i).getBounds(box);
            bounds[i] = box;
        }
        BVH bvh = new BVH(bounds, 3);
        int[] hits = new int[colliders.size()];
        for (int i = 0; i < colliders.size(); i++) {
            int count = bvh.query(bounds[i], hits);
            Collider3D a = colliders.get(bvh.indices[i]);
            for (int j = 0; j < count; j++) {
                a.collide(colliders.get(bvh.indices[hits[j]]), dt);
            }
        }
    }

}
