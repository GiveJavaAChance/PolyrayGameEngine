package polyray.physics;

import polyray.Vector2d;

public class Particle2D extends PhysicsObject2D {

    public final double mass;
    public final double friction;
    public final double restitution;
    private final double invMass;

    public Particle2D(double mass, double friction, double restitution) {
        super();
        this.mass = mass;
        this.invMass = 1.0d / mass;
        this.friction = 1.0d - friction;
        this.restitution = restitution;
    }

    public Particle2D(double mass, Vector2d pos, double friction, double restitution) {
        super(pos);
        this.mass = mass;
        this.invMass = 1.0d / mass;
        this.friction = 1.0d - friction;
        this.restitution = restitution;
    }

    public void applyForce(Vector2d f) {
        this.acc.x += f.x * invMass;
        this.acc.y += f.y * invMass;
    }

    public void applyForce(double fx, double fy) {
        this.acc.x += fx * invMass;
        this.acc.y += fy * invMass;
    }

    public void resolvePenetration(Vector2d normal, double penetrationDepth) {
        pos.x += normal.x * penetrationDepth;
        pos.y += normal.y * penetrationDepth;
        double vx = pos.x - prevPos.x;
        double vy = pos.y - prevPos.y;
        double height = -(vx * normal.x + vy * normal.y);
        if (height <= 0.0d) {
            return;
        }
        double nvx = normal.x * height;
        double nvy = normal.y * height;
        double rx = (vx + nvx) * friction + nvx * restitution;
        double ry = (vy + nvy) * friction + nvy * restitution;
        prevPos.x = pos.x - rx;
        prevPos.y = pos.y - ry;
    }
}
