package polyray.systems.entity;

import java.util.ArrayList;
import polyray.Vector3d;
import polyray.physics.PhysicsObject3D;
import polyray.systems.BVH;
import polyray.physics.Collider3D;
import polyray.physics.CollisionInfo3D;

public class Physics3DSystem {

    private static final ArrayList<PhysicsObject3D> objects = new ArrayList<>();
    
    private static final ArrayList<Controller3D> controllers = new ArrayList<>();

    private static final ArrayList<EntityCollider3D> dynamicColliders = new ArrayList<>();
    private static float[][] dynamicBounds = new float[0][6];

    private static final ArrayList<Collider3D> staticColliders = new ArrayList<>();
    private static float[][] staticBounds = new float[0][6];
    private static BVH staticBVH;
    private static boolean dirtyStatic;
    
    private static double resolvingStrength = 0.5d;
    
    public static final void setResolvingStrength(double strength) {
        resolvingStrength = strength;
    }

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
        Controller3D co = e.getComponent(Controller3D.class);
        if (co != null) {
            co.pos = obj.pos;
            co.prevPos = obj.prevPos;
            controllers.add(co);
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
        Controller3D co = e.getComponent(Controller3D.class);
        if (co != null) {
            controllers.remove(co);
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
        for (Controller3D c : controllers) {
            c.update(dt);
        }
        if (dynamicColliders.isEmpty()) {
            return;
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
        ArrayList<Collision3D> collisions = new ArrayList<>();
        BVH dynamicBVH = new BVH(dynamicBounds, 3);
        float[] query = new float[6];
        int[] hits = new int[40];
        int aIdx = 0;
        for (EntityCollider3D a : dynamicColliders) {
            a.impl.getBounds(query);
            int count = dynamicBVH.query(query, hits);
            if (count != 0) {
                for (int i = 0; i < count; i++) {
                    int bIdx = dynamicBVH.indices[hits[i]];
                    if (aIdx >= bIdx) {
                        continue;
                    }
                    EntityCollider3D b = dynamicColliders.get(bIdx);
                    CollisionInfo3D c = a.impl.collide(b.impl, dt);
                    if (c == null) {
                        continue;
                    }
                    collisions.add(new Collision3D(a, b, null, c));
                }
            }
            count = staticBVH.query(query, hits);
            if (count != 0) {
                for (int i = 0; i < count; i++) {
                    int bIdx = staticBVH.indices[hits[i]];
                    Collider3D b = staticColliders.get(bIdx);
                    CollisionInfo3D c = a.impl.collide(b, dt);
                    if (c == null) {
                        continue;
                    }
                    collisions.add(new Collision3D(a, null, b, c));
                }
            }
            aIdx++;
        }
        for (Collision3D col : collisions) {
            EntityCollider3D a = col.a;
            PhysicsObject3D objA = a.obj;
            Vector3d posA = objA.pos;
            Vector3d prevPosA = objA.prevPos;

            EntityCollider3D b = col.b;

            CollisionInfo3D info = col.c;
            Vector3d normal = info.normal;
            double penetrationDepth = info.penetrationDepth * resolvingStrength;
            if (b != null) {
                penetrationDepth *= 0.5d;
            }
            double dx = normal.x * penetrationDepth;
            double dy = normal.y * penetrationDepth;
            double dz = normal.z * penetrationDepth;
            if (b == null) {
                double vx = posA.x - prevPosA.x;
                double vy = posA.y - prevPosA.y;
                double vz = posA.z - prevPosA.z;
                posA.x += dx;
                posA.y += dy;
                posA.z += dz;
                double height = -(vx * normal.x + vy * normal.y + vz * normal.z);
                if (height < 0.0d) {
                    return;
                }
                double nvx = normal.x * height;
                double nvy = normal.y * height;
                double nvz = normal.z * height;
                Collider3D ac = a.impl;
                double friction = ac.friction;
                double restitution = ac.restitution;
                double rx = (vx + nvx) * friction + nvx * restitution;
                double ry = (vy + nvy) * friction + nvy * restitution;
                double rz = (vz + nvz) * friction + nvz * restitution;
                prevPosA.x = posA.x - rx;
                prevPosA.y = posA.y - ry;
                prevPosA.z = posA.z - rz;
            } else {
                posA.x += dx;
                posA.y += dy;
                posA.z += dz;
                Vector3d posB = b.obj.pos;
                posB.x -= dx;
                posB.y -= dy;
                posB.z -= dz;
            }
        }
        for (EntityCollider3D col : dynamicColliders) {
            col.update();
        }
    }

    private static class Collision3D {

        public final EntityCollider3D a, b;
        public final Collider3D bc;
        public final CollisionInfo3D c;

        public Collision3D(EntityCollider3D a, EntityCollider3D b, Collider3D bc, CollisionInfo3D c) {
            this.a = a;
            this.b = b;
            this.bc = bc;
            this.c = c;
        }
    }
}
