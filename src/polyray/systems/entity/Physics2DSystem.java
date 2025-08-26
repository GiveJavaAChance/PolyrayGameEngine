package polyray.systems.entity;

import java.util.ArrayList;
import polyray.physics.PhysicsObject2D;
import polyray.systems.BVH;
import polyray.physics.Collider2D;
import polyray.physics.CollisionInfo2D;

public class Physics2DSystem {

    private static final ArrayList<PhysicsObject2D> objects = new ArrayList<>();

    private static final ArrayList<EntityCollider2D> dynamicColliders = new ArrayList<>();
    private static float[][] dynamicBounds = new float[0][4];

    private static final ArrayList<Collider2D> staticColliders = new ArrayList<>();
    private static float[][] staticBounds = new float[0][4];
    private static BVH staticBVH;
    private static boolean dirtyStatic;

    public static final void addEntity(Entity e) {
        Collider2D col = e.getComponent(Collider2D.class);
        if (col != null) {
            staticColliders.add(col);
            return;
        }
        PhysicsObject2D obj = e.getComponent(PhysicsObject2D.class);
        if (obj == null) {
            return;
        }
        objects.add(obj);
        EntityCollider2D c = e.getComponent(EntityCollider2D.class);
        if (c != null) {
            c.obj = obj;
            dynamicColliders.add(c);
        }
    }

    public static final void removeEntity(Entity e) {
        Collider2D col = e.getComponent(Collider2D.class);
        if (col != null) {
            staticColliders.remove(col);
            return;
        }
        PhysicsObject2D obj = e.getComponent(PhysicsObject2D.class);
        if (obj == null) {
            return;
        }
        objects.remove(obj);
        EntityCollider2D c = e.getComponent(EntityCollider2D.class);
        if (c != null) {
            dynamicColliders.remove(c);
        }
    }

    public static final void addObject(PhysicsObject2D object) {
        objects.add(object);
    }

    public static final void removeObject(PhysicsObject2D object) {
        objects.remove(object);
    }

    public static final void addCollider(EntityCollider2D collider) {
        dynamicColliders.add(collider);
    }

    public static final void removeCollider(EntityCollider2D collider) {
        dynamicColliders.remove(collider);
    }

    public static final void addStaticCollider(Collider2D collider) {
        staticColliders.add(collider);
        dirtyStatic = true;
    }

    public static final void removeStaticCollider(Collider2D collider) {
        staticColliders.remove(collider);
        dirtyStatic = true;
    }

    public static final void refreshStaticColliders() {
        dirtyStatic = false;
        if (staticBounds.length != staticColliders.size()) {
            staticBounds = new float[staticColliders.size()][4];
        }
        int idx = 0;
        for (Collider2D col : staticColliders) {
            col.getBounds(staticBounds[idx++]);
        }
        staticBVH = new BVH(staticBounds, 2);
    }

    public static final void physicsUpdate(double dt) {
        for (PhysicsObject2D obj : objects) {
            obj.update(dt);
        }
        for (EntityCollider2D col : dynamicColliders) {
            col.update();
        }
        if (dirtyStatic) {
            refreshStaticColliders();
        }
        if (dynamicBounds.length != dynamicColliders.size()) {
            dynamicBounds = new float[dynamicColliders.size()][4];
        }
        int idx = 0;
        for (EntityCollider2D col : dynamicColliders) {
            col.impl.getBounds(dynamicBounds[idx++]);
        }
        BVH dynamicBVH = new BVH(dynamicBounds, 2);
        float[] query = new float[4];
        int[] hits = new int[100];
        int aIdx = 0;
        for (EntityCollider2D a : dynamicColliders) {
            a.impl.getBounds(query);
            int count = dynamicBVH.query(query, hits);
            if (count != 0) {
                for (int i = 0; i < count; i++) {
                    int bIdx = dynamicBVH.indices[hits[i]];
                    if (aIdx == bIdx) {
                        continue;
                    }
                    EntityCollider2D b = dynamicColliders.get(bIdx);
                    CollisionInfo2D c = a.impl.collide(b.impl, dt);
                    if (c != null) {
                        // Handle, ooorrr, maybe defer for multithreading?
                    }
                }
            }
            count = staticBVH.query(query, hits);
            if (count != 0) {
                for (int i = 0; i < count; i++) {
                    int bIdx = staticBVH.indices[hits[i]];
                    Collider2D b = staticColliders.get(bIdx);
                    CollisionInfo2D c = a.impl.collide(b, dt);
                    if (c != null) {
                        // Handle, ooorrr, maybe defer for multithreading?
                    }
                }
            }
            aIdx++;
        }
    }

}
