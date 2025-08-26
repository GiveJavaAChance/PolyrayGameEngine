package polyray.systems.entity;

import polyray.Vector2d;
import polyray.physics.Collider2D;
import polyray.physics.PhysicsObject2D;

public final class EntityCollider2D {

    protected final Collider2D impl;
    protected final Vector2d offset;
    protected PhysicsObject2D obj;
    
    public EntityCollider2D(Collider2D impl, Vector2d offset) {
        this.impl = impl;
        this.offset = offset;
    }
    
    public final void update() {
        impl.pos.x = obj.pos.x + offset.x;
        impl.pos.y = obj.pos.y + offset.y;
    }
}
