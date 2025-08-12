package polyray.systems.entity;

import polyray.Vector2d;
import polyray.physics.Collider2D;

public abstract class EntityCollider2D implements Component, Collider2D {

    public final Vector2d pos, prevPos;

    public EntityCollider2D(Vector2d pos, Vector2d prevPos) {
        this.pos = pos;
        this.prevPos = prevPos;
    }
}
