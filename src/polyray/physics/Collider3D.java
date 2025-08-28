package polyray.physics;

import polyray.Vector3d;

public abstract class Collider3D {

    public final Vector3d pos, size;
    public double friction, restitution;

    public Collider3D(Vector3d pos, Vector3d size) {
        this.pos = pos;
        this.size = size;
    }

    public Collider3D(Vector3d pos, Vector3d size, double friction, double restitution) {
        this(pos, size);
        this.friction = 1.0d - friction;
        this.restitution = restitution;
    }

    public final void getBounds(float[] box) {
        box[0] = (float) pos.x;
        box[1] = (float) pos.y;
        box[2] = (float) pos.z;
        box[3] = (float) (pos.x + size.x);
        box[4] = (float) (pos.y + size.y);
        box[5] = (float) (pos.z + size.z);
    }

    public abstract CollisionInfo3D collide(Collider3D other, double dt);

}
