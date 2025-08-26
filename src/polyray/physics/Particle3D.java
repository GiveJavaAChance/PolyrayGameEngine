package polyray.physics;

import polyray.Vector3d;

public class Particle3D extends PhysicsObject3D {

    public final double mass;
    public final double friction;
    public final double restitution;
    private final double invMass;

    public Particle3D(double mass, double friction, double restitution) {
        super();
        this.mass = mass;
        this.invMass = 1.0d / mass;
        this.friction = 1.0d - friction;
        this.restitution = restitution;
    }

    public Particle3D(double mass, Vector3d pos, double friction, double restitution) {
        super(pos);
        this.mass = mass;
        this.invMass = 1.0d / mass;
        this.friction = 1.0d - friction;
        this.restitution = restitution;
    }

    public void applyForce(Vector3d f) {
        this.acc.x += f.x * invMass;
        this.acc.y += f.y * invMass;
        this.acc.z += f.z * invMass;
    }

    public void applyForce(double fx, double fy, double fz) {
        this.acc.x += fx * invMass;
        this.acc.y += fy * invMass;
        this.acc.z += fz * invMass;
    }

    public void resolvePenetration(Vector3d normal, double penetrationDepth) {
        pos.x += normal.x * penetrationDepth;
        pos.y += normal.y * penetrationDepth;
        pos.z += normal.z * penetrationDepth;
        double vx = pos.x - prevPos.x;
        double vy = pos.y - prevPos.y;
        double vz = pos.z - prevPos.z;
        double height = -(vx * normal.x + vy * normal.y + vz * normal.z);
        if (height <= 0.0d) {
            return;
        }
        double nvx = normal.x * height;
        double nvy = normal.y * height;
        double nvz = normal.z * height;
        double rx = (vx + nvx) * friction + nvx * restitution;
        double ry = (vy + nvy) * friction + nvy * restitution;
        double rz = (vz + nvz) * friction + nvz * restitution;
        prevPos.x = pos.x - rx;
        prevPos.y = pos.y - ry;
        prevPos.z = pos.z - rz;
    }
}
