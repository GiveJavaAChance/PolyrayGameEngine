package polyray.systems.entity;

import java.util.ArrayList;
import polyray.physics.PhysicsObject3D;
import polyray.systems.BVH;
import polyray.physics.Collider3D;
import polyray.physics.CollisionInfo3D;

public class Physics3DSystem {

    private static final ArrayList<PhysicsObject3D> objects = new ArrayList<>();

    private static final ArrayList<EntityCollider3D> dynamicColliders = new ArrayList<>();
    private static float[][] dynamicBounds = new float[0][6];

    private static final ArrayList<Collider3D> staticColliders = new ArrayList<>();
    private static float[][] staticBounds = new float[0][6];
    private static BVH staticBVH;
    private static boolean dirtyStatic;

    public static final void addEntity(Entity e) {
        Collider3D col = e.getComponent(Collider3D.class);
        if (col != null) {
            staticColliders.add(col);
            return;
        }
        PhysicsObject3D obj = e.getComponent(PhysicsObject3D.class);
        if (obj == null) {
            return;
        }
        objects.add(obj);
        EntityCollider3D c = e.getComponent(EntityCollider3D.class);
        if (c != null) {
            c.obj = obj;
            dynamicColliders.add(c);
        }
    }

    public static final void removeEntity(Entity e) {
        Collider3D col = e.getComponent(Collider3D.class);
        if (col != null) {
            staticColliders.remove(col);
            return;
        }
        PhysicsObject3D obj = e.getComponent(PhysicsObject3D.class);
        if (obj == null) {
            return;
        }
        objects.remove(obj);
        EntityCollider3D c = e.getComponent(EntityCollider3D.class);
        if (c != null) {
            dynamicColliders.remove(c);
        }
    }

    public static final void addObject(PhysicsObject3D object) {
        objects.add(object);
    }

    public static final void removeObject(PhysicsObject3D object) {
        objects.remove(object);
    }

    public static final void addCollider(EntityCollider3D collider) {
        dynamicColliders.add(collider);
    }

    public static final void removeCollider(EntityCollider3D collider) {
        dynamicColliders.remove(collider);
    }

    public static final void addStaticCollider(Collider3D collider) {
        staticColliders.add(collider);
        dirtyStatic = true;
    }

    public static final void removeStaticCollider(Collider3D collider) {
        staticColliders.remove(collider);
        dirtyStatic = true;
    }

    public static final void refreshStaticColliders() {
        dirtyStatic = false;
        if (staticBounds.length != staticColliders.size()) {
            staticBounds = new float[staticColliders.size()][6];
        }
        int idx = 0;
        for (Collider3D col : staticColliders) {
            col.getBounds(staticBounds[idx++]);
        }
        staticBVH = new BVH(staticBounds, 3);
    }

    public static final void physicsUpdate(double dt) {
        for (PhysicsObject3D obj : objects) {
            obj.update(dt);
        }
        for (EntityCollider3D col : dynamicColliders) {
            col.update();
        }
        if (dirtyStatic) {
            refreshStaticColliders();
        }
        if (dynamicBounds.length != dynamicColliders.size()) {
            dynamicBounds = new float[dynamicColliders.size()][6];
        }
        int idx = 0;
        for (EntityCollider3D col : dynamicColliders) {
            col.impl.getBounds(dynamicBounds[idx++]);
        }
        BVH dynamicBVH = new BVH(dynamicBounds, 3);
        float[] query = new float[6];
        int[] hits = new int[100];
        int aIdx = 0;
        for (EntityCollider3D a : dynamicColliders) {
            a.impl.getBounds(query);
            int count = dynamicBVH.query(query, hits);
            if (count != 0) {
                for (int i = 0; i < count; i++) {
                    int bIdx = dynamicBVH.indices[hits[i]];
                    if (aIdx == bIdx) {
                        continue;
                    }
                    EntityCollider3D b = dynamicColliders.get(bIdx);
                    CollisionInfo3D c = a.impl.collide(b.impl, dt);
                    if (c != null) {
                        // Handle, ooorrr, maybe defer for multithreading?
                    }
                }
            }
            count = staticBVH.query(query, hits);
            if (count != 0) {
                for (int i = 0; i < count; i++) {
                    int bIdx = staticBVH.indices[hits[i]];
                    Collider3D b = staticColliders.get(bIdx);
                    CollisionInfo3D c = a.impl.collide(b, dt);
                    if (c != null) {
                        // Handle, ooorrr, maybe defer for multithreading?
                    }
                }
            }
            aIdx++;
        }
    }
}
