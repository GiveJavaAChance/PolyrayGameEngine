package polyray.physics;

import polyray.Vector2d;

public class PhysicsObject2D {

    public final Vector2d pos = new Vector2d();
    public final Vector2d prevPos = new Vector2d();
    public final Vector2d acc = new Vector2d();

    public PhysicsObject2D() {
    }

    public PhysicsObject2D(Vector2d pos) {
        this.pos.x = pos.x;
        this.pos.y = pos.y;
        this.prevPos.x = pos.x;
        this.prevPos.y = pos.y;
    }

    public void accelerate(Vector2d a) {
        acc.x += a.x;
        acc.y += a.y;
    }

    public void update(double dt) {
        double tx = pos.x;
        double ty = pos.y;
        double ddt = dt * dt;
        pos.x += pos.x - prevPos.x + acc.x * ddt;
        pos.y += pos.y - prevPos.y + acc.y * ddt;
        prevPos.x = tx;
        prevPos.y = ty;
        acc.x = 0.0d;
        acc.y = 0.0d;
    }
}
