package polyray.physics;

import polyray.Vector2d;

public class ParticleLineN2D {

    private final Particle2D[] particles;
    private final double massSum;
    private final double[] segmentLengths;
    public final Vector2d lineOrigin = new Vector2d();
    public final Vector2d lineVector = new Vector2d();

    public ParticleLineN2D(double[] segmentLengths, Particle2D[] particles) {
        this.segmentLengths = segmentLengths;
        this.particles = particles;
        double sum = 0.0d;
        for (Particle2D p : particles) {
            sum += p.mass;
        }
        this.massSum = sum;
    }

    public void update() {
        lineOrigin.x = 0.0d;
        lineOrigin.y = 0.0d;
        for (Particle2D p : particles) {
            lineOrigin.x += p.pos.x * p.mass;
            lineOrigin.y += p.pos.y * p.mass;
        }
        lineOrigin.x /= massSum;
        lineOrigin.y /= massSum;

        lineVector.x = 0.0d;
        lineVector.y = 0.0d;
        for (Particle2D p : particles) {
            double dx = (p.pos.x - lineOrigin.x) * p.mass;
            double dy = (p.pos.y - lineOrigin.y) * p.mass;
            if (dx * lineVector.x + dy * lineVector.y < 0.0d) {
                lineVector.x -= dx;
                lineVector.y -= dy;
            } else {
                lineVector.x += dx;
                lineVector.y += dy;
            }
        }
        double mul = 1.0d / Vector2d.length(lineVector);
        lineVector.x *= mul;
        lineVector.y *= mul;

        for (Particle2D p : particles) {
            double dx = p.pos.x - lineOrigin.x;
            double dy = p.pos.y - lineOrigin.y;
            double t = dx * lineVector.x + dy * lineVector.y;
            p.pos.x = lineOrigin.x + lineVector.x * t;
            p.pos.y = lineOrigin.y + lineVector.y * t;
        }
        for (int j = 0; j < 10; j++) {
            Particle2D a = particles[0];
            for (int i = 1; i < particles.length; i++) {
                Particle2D b = particles[i];
                applyLine(a, b, (Vector2d.dot(Vector2d.sub(b.pos, a.pos), lineVector) + segmentLengths[i - 1]) * 0.5d);
                a = b;
            }
        }
    }

    private void applyLine(Particle2D a, Particle2D b, double diff) {
        double ratio1 = a.mass / (a.mass + b.mass);
        double ratio2 = ratio1 - 1.0d;
        double dx = lineVector.x * diff;
        double dy = lineVector.y * diff;
        a.pos.x += dx * ratio1;
        a.pos.y += dy * ratio1;
        b.pos.x += dx * ratio2;
        b.pos.y += dy * ratio2;
    }
}
