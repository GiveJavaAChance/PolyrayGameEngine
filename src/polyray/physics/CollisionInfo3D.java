package polyray.physics;

import polyray.Vector3d;

public final class CollisionInfo3D {

    public final Vector3d normal;
    public final double penetrationDepth;

    public CollisionInfo3D(Vector3d normal, double penetrationDepth) {
        this.normal = normal;
        this.penetrationDepth = penetrationDepth;
    }
}
