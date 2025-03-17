package polyray.physics;

import polyray.Vector3d;

public class ParticleLine {

    public final Particle a, b;
    public final double length;
    private final double stiffness;
    private final double ratioA, ratioB;

    public ParticleLine(Particle a, Particle b, double length, double stiffness) {
        this.a = a;
        this.b = b;
        this.length = length;
        this.stiffness = stiffness * 0.5d;
        double massSum = a.mass + b.mass;
        this.ratioA = a.mass / massSum;
        this.ratioB = b.mass / massSum;
    }

    public void applyRotationalForce(Vector3d axis, double force) {
        Vector3d midpoint = new Vector3d(
                a.pos.x * ratioA + b.pos.x * ratioB,
                a.pos.y * ratioA + b.pos.y * ratioB,
                a.pos.z * ratioA + b.pos.z * ratioB
        );
        
        // This does not account for particle mass but is enough for now
        Vector3d ra = new Vector3d(a.pos.x - midpoint.x, a.pos.y - midpoint.y, a.pos.z - midpoint.z);
        Vector3d rb = new Vector3d(b.pos.x - midpoint.x, b.pos.y - midpoint.y, b.pos.z - midpoint.z);

        Vector3d torqueA = Vector3d.mul(Vector3d.cross(ra, axis), force);
        Vector3d torqueB = Vector3d.mul(Vector3d.cross(rb, axis), force);

        a.applyForce(torqueA);
        b.applyForce(torqueB);
    }

    public void update() {
        double dx = b.pos.x - a.pos.x;
        double dy = b.pos.y - a.pos.y;
        double dz = b.pos.z - a.pos.z;
        double dist = Math.sqrt(dx * dx + dy * dy + dz * dz);
        double diff = (dist - length) * stiffness;
        double mul = diff / dist;
        dx *= mul;
        dy *= mul;
        dz *= mul;
        double sum = a.mass + b.mass;
        double ratio1 = a.mass / sum;
        double ratio2 = b.mass / sum;
        a.pos.x += dx * ratio1;
        a.pos.y += dy * ratio1;
        a.pos.z += dz * ratio1;
        b.pos.x -= dx * ratio2;
        b.pos.y -= dy * ratio2;
        b.pos.z -= dz * ratio2;
    }

    public static final ParticleLine[] connectAllLines(Particle p, Particle[] particles, double stiffness) {
        ParticleLine[] lines = new ParticleLine[particles.length];
        for (int i = 0; i < particles.length; i++) {
            lines[i] = new ParticleLine(p, particles[i], Vector3d.length(Vector3d.sub(p.pos, particles[i].pos)), stiffness);
        }
        return lines;
    }
}
