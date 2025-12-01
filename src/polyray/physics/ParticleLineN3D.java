package polyray.physics;

import polyray.Vector3d;

public class ParticleLineN3D {

    private final Particle3D[] particles;
    private final double massSum;
    private final double[] segmentLengths;
    public final Vector3d lineOrigin = new Vector3d();
    public final Vector3d lineVector = new Vector3d();

    public ParticleLineN3D(double[] segmentLengths, Particle3D[] particles) {
        this.segmentLengths = segmentLengths;
        this.particles = particles;
        double sum = 0.0d;
        for (Particle3D p : particles) {
            sum += p.mass;
        }
        this.massSum = sum;
    }

    public void update() {
        lineOrigin.x = 0.0d;
        lineOrigin.y = 0.0d;
        lineOrigin.z = 0.0d;
        for (Particle3D p : particles) {
            lineOrigin.x += p.pos.x * p.mass;
            lineOrigin.y += p.pos.y * p.mass;
            lineOrigin.z += p.pos.z * p.mass;
        }
        lineOrigin.x /= massSum;
        lineOrigin.y /= massSum;
        lineOrigin.z /= massSum;

        lineVector.x = 0.0d;
        lineVector.y = 0.0d;
        lineVector.z = 0.0d;
        for (Particle3D p : particles) {
            double dx = (p.pos.x - lineOrigin.x) * p.mass;
            double dy = (p.pos.y - lineOrigin.y) * p.mass;
            double dz = (p.pos.z - lineOrigin.z) * p.mass;
            if (dx * lineVector.x + dy * lineVector.y + dz * lineVector.z < 0.0d) {
                lineVector.x -= dx;
                lineVector.y -= dy;
                lineVector.z -= dz;
            } else {
                lineVector.x += dx;
                lineVector.y += dy;
                lineVector.z += dz;
            }
        }
        double mul = 1.0d / Vector3d.length(lineVector);
        lineVector.x *= mul;
        lineVector.y *= mul;
        lineVector.z *= mul;
        for (Particle3D p : particles) {
            double dx = p.pos.x - lineOrigin.x;
            double dy = p.pos.y - lineOrigin.y;
            double dz = p.pos.z - lineOrigin.z;
            double t = dx * lineVector.x + dy * lineVector.y + dz * lineVector.z;
            p.pos.x = lineOrigin.x + lineVector.x * t;
            p.pos.y = lineOrigin.y + lineVector.y * t;
            p.pos.z = lineOrigin.z + lineVector.z * t;
        }
        for (int j = 0; j < 10; j++) {
            Particle3D a = particles[0];
            for (int i = 1; i < particles.length; i++) {
                Particle3D b = particles[i];
                applyLine(a, b, (Vector3d.dot(Vector3d.sub(b.pos, a.pos), lineVector) + segmentLengths[i - 1]) * 0.5d);
                a = b;
            }
        }
    }

    private void applyLine(Particle3D a, Particle3D b, double diff) {
        double ratio1 = a.mass / (a.mass + b.mass);
        double ratio2 = ratio1 - 1.0d;
        double dx = lineVector.x * diff;
        double dy = lineVector.y * diff;
        double dz = lineVector.z * diff;
        a.pos.x += dx * ratio1;
        a.pos.y += dy * ratio1;
        a.pos.z += dz * ratio1;
        b.pos.x += dx * ratio2;
        b.pos.y += dy * ratio2;
        b.pos.z += dz * ratio2;
    }
}
