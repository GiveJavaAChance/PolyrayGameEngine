package polyray.systems.entity;

import polyray.Vector3d;
import polyray.physics.Collider3D;
import polyray.physics.PhysicsObject3D;

public final class EntityCollider3D {

    protected final Collider3D impl;
    protected final Vector3d offset;
    protected PhysicsObject3D obj;

    public EntityCollider3D(Collider3D impl, Vector3d offset) {
        this.impl = impl;
        this.offset = offset;
    }

    public final void update() {
        impl.pos.x = obj.pos.x + offset.x;
        impl.pos.y = obj.pos.y + offset.y;
        impl.pos.z = obj.pos.z + offset.z;
    }
}
