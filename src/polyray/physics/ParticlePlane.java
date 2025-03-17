package polyray.physics;

import polyray.Vector3d;

public class ParticlePlane {

    private final Particle[] particles;
    private final Vector3d[] pointVectors;
    private final double invAmt;
    public final Vector3d planeOrigin = new Vector3d();
    public final Vector3d planeNormal = new Vector3d();

    public ParticlePlane(Particle[] particles) {
        this.particles = particles;
        this.pointVectors = new Vector3d[particles.length];
        this.invAmt = 1.0d / particles.length;
    }

    public void update() {
        planeOrigin.x = 0.0d;
        planeOrigin.y = 0.0d;
        planeOrigin.z = 0.0d;
        for (Particle p : particles) {
            planeOrigin.x += p.pos.x;
            planeOrigin.y += p.pos.y;
            planeOrigin.z += p.pos.z;
        }
        planeOrigin.x *= invAmt;
        planeOrigin.y *= invAmt;
        planeOrigin.z *= invAmt;

        int i = 0;
        planeNormal.x = 0.0d;
        planeNormal.y = 0.0d;
        planeNormal.z = 0.0d;
        Vector3d prevPointVector = Vector3d.sub(particles[particles.length - 1].pos, planeOrigin);
        for (Particle p : particles) {
            Vector3d pointVector = Vector3d.sub(p.pos, planeOrigin);
            pointVectors[i++] = pointVector;
            Vector3d normal = Vector3d.cross(pointVector, prevPointVector);
            double weight = Vector3d.length(normal);
            if (Vector3d.dot(normal, planeNormal) < 0.0d) {
                planeNormal.x -= normal.x * weight;
                planeNormal.y -= normal.y * weight;
                planeNormal.z -= normal.z * weight;
            } else {
                planeNormal.x += normal.x * weight;
                planeNormal.y += normal.y * weight;
                planeNormal.z += normal.z * weight;
            }
            prevPointVector = pointVector;
        }
        double mul = 1.0d / Vector3d.length(planeNormal);
        planeNormal.x *= mul;
        planeNormal.y *= mul;
        planeNormal.z *= mul;

        i = 0;
        for (Particle p : particles) {
            double height = Vector3d.dot(pointVectors[i++], planeNormal);
            Vector3d correction = Vector3d.mul(planeNormal, -height);
            p.pos.x += correction.x;
            p.pos.y += correction.y;
            p.pos.z += correction.z;
        }
    }
}
