package polyray.physics;

import polyray.Vector3d;
import polyray.Vector3f;

public class PhysicsObject {

    public final Vector3d pos = new Vector3d();
    public final Vector3d prevPos = new Vector3d();
    public final Vector3d acc = new Vector3d();
    public final float friction, restitution;

    public PhysicsObject(float friction, float restitution) {
        this.friction = friction;
        this.restitution = restitution;
    }

    public PhysicsObject(Vector3d pos, float friction, float restitution) {
        this.pos.x = pos.x;
        this.pos.y = pos.y;
        this.pos.z = pos.z;
        this.prevPos.x = pos.x;
        this.prevPos.y = pos.y;
        this.prevPos.z = pos.z;
        this.friction = friction;
        this.restitution = restitution;
    }

    public void accelerate(Vector3d a) {
        acc.x += a.x;
        acc.y += a.y;
        acc.z += a.z;
    }

    public void update(double dt) {
        double tx = pos.x;
        double ty = pos.y;
        double tz = pos.z;
        double ddt = dt * dt;
        pos.x += pos.x - prevPos.x + acc.x * ddt;
        pos.y += pos.y - prevPos.y + acc.y * ddt;
        pos.z += pos.z - prevPos.z + acc.z * ddt;
        prevPos.x = tx;
        prevPos.y = ty;
        prevPos.z = tz;
        acc.x = 0.0d;
        acc.y = 0.0d;
        acc.z = 0.0d;
    }

    protected Vector3f bounce(Vector3f v, Vector3f n) {
        float height = -Vector3f.dot(v, n);
        Vector3f normVel = Vector3f.mul(n, height);
        Vector3f perp = Vector3f.mul(Vector3f.add(v, normVel), 1.0f - friction);
        return Vector3f.add(perp, Vector3f.mul(normVel, restitution));
    }
}
