package polyray.physics;

import polyray.Vector2d;

public class ParticleLineN2D {

    private final Particle2D[] particles;
    private final Vector2d[] pointVectors;
    private final double invAmt;
    private final double[] segmentLengths;
    public final Vector2d lineOrigin = new Vector2d();
    public final Vector2d lineVector = new Vector2d();

    public ParticleLineN2D(double[] segmentLengths, Particle2D[] particles) {
        this.segmentLengths = segmentLengths;
        this.particles = particles;
        this.pointVectors = new Vector2d[particles.length];
        this.invAmt = 1.0d / particles.length;
    }

    public void update() {
        lineOrigin.x = 0.0d;
        lineOrigin.y = 0.0d;
        for (Particle2D p : particles) {
            lineOrigin.x += p.pos.x;
            lineOrigin.y += p.pos.y;
        }
        lineOrigin.x *= invAmt;
        lineOrigin.y *= invAmt;

        lineVector.x = 0.0d;
        lineVector.y = 0.0d;
        int i = 0;
        for (Particle2D p : particles) {
            Vector2d dir = Vector2d.sub(lineOrigin, p.pos);
            pointVectors[i++] = dir;
            if (Vector2d.dot(lineVector, dir) < 0.0d) {
                lineVector.x -= dir.x;
                lineVector.y -= dir.y;
            } else {
                lineVector.x += dir.x;
                lineVector.y += dir.y;
            }
        }
        double mul = 1.0d / Vector2d.length(lineVector);
        lineVector.x *= mul;
        lineVector.y *= mul;

        i = 0;
        for (Particle2D p : particles) {
            Vector2d pointVector = pointVectors[i++];
            Vector2d lineNormal = Vector2d.normalize(new Vector2d(-pointVector.y, pointVector.x));
            double dist = Vector2d.dot(pointVector, lineNormal);
            if (Math.abs(dist) < 0.0001d) {
                continue;
            }
            Vector2d correction = Vector2d.mul(lineNormal, dist);
            p.pos.x += correction.x;
            p.pos.y += correction.y;
        }
        for (int j = 0; j < 10; j++) {
            Particle2D a = particles[0];
            for (i = 1; i < particles.length; i++) {
                Particle2D b = particles[i];
                applyLine(lineVector, a, b, (Vector2d.dot(Vector2d.sub(b.pos, a.pos), lineVector) - segmentLengths[i - 1]) * 0.5d);
                a = b;
            }
        }
    }

    private static void applyLine(Vector2d dir, Particle2D a, Particle2D b, double diff) {
        double sum = a.mass + b.mass;
        double ratio1 = a.mass / sum;
        double ratio2 = b.mass / sum;
        Vector2d ca = Vector2d.mul(dir, diff * ratio1);
        Vector2d cb = Vector2d.mul(dir, -diff * ratio2);
        a.pos.x += ca.x;
        a.pos.y += ca.y;
        b.pos.x += cb.x;
        b.pos.y += cb.y;
    }
}
