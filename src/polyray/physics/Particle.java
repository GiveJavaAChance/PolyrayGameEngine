package polyray.physics;

import polyray.Vector3d;

public class Particle extends PhysicsObject {

    public final double mass;
    private final double invMass;

    public Particle(double mass, float friction, float restitution) {
        super(friction, restitution);
        this.mass = mass;
        this.invMass = 1.0d / mass;
    }

    public Particle(double mass, Vector3d pos, float friction, float restitution) {
        super(pos, friction, restitution);
        this.mass = mass;
        this.invMass = 1.0d / mass;
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

}
