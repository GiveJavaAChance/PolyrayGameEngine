package polyray.systems.entity;

import polyray.Vector3d;
import polyray.physics.Collider3D;

public abstract class EntityCollider3D implements Component, Collider3D {

    public final Vector3d pos, prevPos;

    public EntityCollider3D(Vector3d pos, Vector3d prevPos) {
        this.pos = pos;
        this.prevPos = prevPos;
    }
}
