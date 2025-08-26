package polyray.physics;

import polyray.Vector2d;
import polyray.Vector3d;

public class ParticleLine2D {

    public final Particle2D a, b;
    public final double length;
    private final double stiffness;
    private final double ratioA, ratioB;

    public ParticleLine2D(Particle2D a, Particle2D b, double length, double stiffness) {
        this.a = a;
        this.b = b;
        this.length = length;
        this.stiffness = stiffness * 0.5d;
        double massSum = a.mass + b.mass;
        this.ratioA = a.mass / massSum;
        this.ratioB = b.mass / massSum;
    }

    public void applyRotationalForce(double force) {
        Vector2d midpoint = new Vector2d(
                a.pos.x * ratioA + b.pos.x * ratioB,
                a.pos.y * ratioA + b.pos.y * ratioB
        );

        Vector2d ra = new Vector2d(midpoint.y - a.pos.y, a.pos.x - midpoint.x);
        Vector2d rb = new Vector2d(midpoint.y - b.pos.y, b.pos.x - midpoint.x);

        Vector2d torqueA = Vector2d.mul(ra, force);
        Vector2d torqueB = Vector2d.mul(rb, force);

        a.applyForce(torqueA);
        b.applyForce(torqueB);
    }

    public void update() {
        double dx = b.pos.x - a.pos.x;
        double dy = b.pos.y - a.pos.y;
        double dist = Math.sqrt(dx * dx + dy * dy);
        double diff = (dist - length) * stiffness;
        double mul = diff / dist;
        dx *= mul;
        dy *= mul;
        a.pos.x += dx * ratioB;
        a.pos.y += dy * ratioB;
        b.pos.x -= dx * ratioA;
        b.pos.y -= dy * ratioA;
    }

    public static final ParticleLine2D[] connectAllLines(Particle2D p, Particle2D[] particles, double stiffness) {
        ParticleLine2D[] lines = new ParticleLine2D[particles.length];
        for (int i = 0; i < particles.length; i++) {
            lines[i] = new ParticleLine2D(p, particles[i], Vector2d.length(Vector2d.sub(p.pos, particles[i].pos)), stiffness);
        }
        return lines;
    }
}
