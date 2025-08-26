package polyray.physics;

import polyray.Vector2d;

public final class CollisionInfo2D {

    public final Vector2d normal;
    public final double penetrationDepth;

    public CollisionInfo2D(Vector2d normal, double penetrationDepth) {
        this.normal = normal;
        this.penetrationDepth = penetrationDepth;
    }
}
