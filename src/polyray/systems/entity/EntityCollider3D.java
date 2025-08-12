package polyray.systems.entity;

import polyray.physics.Collider;
import polyray.Vector3d;

public abstract class EntityCollider3D implements Component, Collider {

    public final Vector3d pos, prevPos;

    public EntityCollider3D(Vector3d pos, Vector3d prevPos) {
        this.pos = pos;
        this.prevPos = prevPos;
    }
}
