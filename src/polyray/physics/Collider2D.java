package polyray.physics;

import polyray.Vector2d;

public abstract class Collider2D {

    public final Vector2d pos, size;
    public double friction, restitution;

    public Collider2D(Vector2d pos, Vector2d size) {
        this.pos = pos;
        this.size = size;
    }
    
    public Collider2D(Vector2d pos, Vector2d size, double friction, double restitution) {
        this(pos, size);
        this.friction = 1.0d - friction;
        this.restitution = restitution;
    }

    public final void getBounds(float[] box) {
        box[0] = (float) pos.x;
        box[1] = (float) pos.y;
        box[2] = (float) (pos.x + size.x);
        box[3] = (float) (pos.y + size.y);
    }

    public abstract CollisionInfo2D collide(Collider2D other, double dt);

}
