package polyray.systems.entity;

import polyray.physics.Collider;
import polyray.Vector2d;

public abstract class EntityCollider2D implements Component, Collider {

    public final Vector2d pos, prevPos;

    public EntityCollider2D(Vector2d pos, Vector2d prevPos) {
        this.pos = pos;
        this.prevPos = prevPos;
    }
}
