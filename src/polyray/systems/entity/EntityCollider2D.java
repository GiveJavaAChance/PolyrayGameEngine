package polyray.systems.entity;

import polyray.Vector2d;
import polyray.physics.Collider2D;

public abstract class EntityCollider2D implements Component, Collider2D {

    protected Vector2d pos, prevPos;

}
