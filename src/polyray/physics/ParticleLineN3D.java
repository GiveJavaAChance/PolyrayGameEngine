package polyray.physics;

import polyray.Vector3d;

public class ParticleLineN3D {

    private final Particle3D[] particles;
    private final Vector3d[] pointVectors;
    private final double invAmt;
    private final double[] segmentLengths;
    public final Vector3d lineOrigin = new Vector3d();
    public final Vector3d lineVector = new Vector3d();

    public ParticleLineN3D(double[] segmentLengths, Particle3D[] particles) {
        this.segmentLengths = segmentLengths;
        this.particles = particles;
        this.pointVectors = new Vector3d[particles.length];
        this.invAmt = 1.0d / particles.length;
    }

    public void update() {
        lineOrigin.x = 0.0d;
        lineOrigin.y = 0.0d;
        lineOrigin.z = 0.0d;
        for (Particle3D p : particles) {
            lineOrigin.x += p.pos.x;
            lineOrigin.y += p.pos.y;
            lineOrigin.z += p.pos.z;
        }
        lineOrigin.x *= invAmt;
        lineOrigin.y *= invAmt;
        lineOrigin.z *= invAmt;

        lineVector.x = 0.0d;
        lineVector.y = 0.0d;
        lineVector.z = 0.0d;
        int i = 0;
        for (Particle3D p : particles) {
            Vector3d dir = Vector3d.sub(lineOrigin, p.pos);
            pointVectors[i++] = dir;
            if (Vector3d.dot(lineVector, dir) < 0.0d) {
                lineVector.x -= dir.x;
                lineVector.y -= dir.y;
                lineVector.z -= dir.z;
            } else {
                lineVector.x += dir.x;
                lineVector.y += dir.y;
                lineVector.z += dir.z;
            }
        }
        double mul = 1.0d / Vector3d.length(lineVector);
        lineVector.x *= mul;
        lineVector.y *= mul;
        lineVector.z *= mul;
        
        i = 0;
        for (Particle3D p : particles) {
            Vector3d pointVector = pointVectors[i++];
            Vector3d lineNormalBitangent = Vector3d.cross(lineVector, pointVector);
            Vector3d lineNormal = Vector3d.normalize(Vector3d.cross(lineVector, lineNormalBitangent));
            double dist = Vector3d.dot(pointVector, lineNormal);
            if (Math.abs(dist) < 0.0001d) {
                continue;
            }
            Vector3d correction = Vector3d.mul(lineNormal, dist);
            p.pos.x += correction.x;
            p.pos.y += correction.y;
            p.pos.z += correction.z;
        }
        for (int j = 0; j < 10; j++) {
            Particle3D a = particles[0];
            for (i = 1; i < particles.length; i++) {
                Particle3D b = particles[i];
                applyLine(lineVector, a, b, (Vector3d.dot(Vector3d.sub(b.pos, a.pos), lineVector) - segmentLengths[i - 1]) * 0.5d);
                a = b;
            }
        }
    }

    private void applyLine(Vector3d dir, Particle3D a, Particle3D b, double diff) {
        double sum = a.mass + b.mass;
        double ratio1 = a.mass / sum;
        double ratio2 = b.mass / sum;
        Vector3d ca = Vector3d.mul(dir, diff * ratio1);
        Vector3d cb = Vector3d.mul(dir, -diff * ratio2);
        a.pos.x += ca.x;
        a.pos.y += ca.y;
        a.pos.z += ca.z;
        b.pos.x += cb.x;
        b.pos.y += cb.y;
        b.pos.z += cb.z;
    }
}
