package polyray.physics;

import polyray.Vector3d;

public class ParticleSphere {

    private final Particle[] particles;
    private final double invAmt;
    private final double radius;
    public final Vector3d sphereOrigin = new Vector3d();

    public ParticleSphere(double radius, Particle... particles) {
        this.radius = radius;
        this.particles = particles;
        this.invAmt = 1.0d / particles.length;
    }

    public void update() {
        sphereOrigin.x = 0.0d;
        sphereOrigin.y = 0.0d;
        sphereOrigin.z = 0.0d;
        for (Particle p : particles) {
            sphereOrigin.x += p.pos.x;
            sphereOrigin.y += p.pos.y;
            sphereOrigin.z += p.pos.z;
        }
        sphereOrigin.x *= invAmt;
        sphereOrigin.y *= invAmt;
        sphereOrigin.z *= invAmt;

        for (Particle p : particles) {
            Vector3d pointVector = Vector3d.sub(p.pos, sphereOrigin);
            double len = Vector3d.length(pointVector);
            double mul = (radius - len) / len;
            pointVector.x *= mul;
            pointVector.y *= mul;
            pointVector.z *= mul;
            p.pos.x += pointVector.x;
            p.pos.y += pointVector.y;
            p.pos.z += pointVector.z;
        }
    }
}
