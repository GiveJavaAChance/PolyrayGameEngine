package polyray.systems.noise;

import java.util.Random;
import polyray.Vector3d;
import polyray.Vector3f;

public class FractalNoise {

    private static final double EPSILON = 1.0d;

    private final double noiseScale;
    private final double heightScale;
    private final double[] sin;
    private final double[] cos;

    public FractalNoise(double noiseScale, double heightScale, int detail, long seed) {
        this.noiseScale = noiseScale;
        this.heightScale = heightScale;
        this.sin = new double[detail];
        this.cos = new double[detail];
        setSeed(seed);
    }

    public final void setSeed(long seed) {
        Random random = new Random(seed);
        System.out.println("Seed: " + seed);
        for (int i = 0; i < sin.length; i++) {
            double ang = random.nextDouble() * Math.PI * 2.0d;
            sin[i] = Math.sin(ang);
            cos[i] = Math.cos(ang);
        }
    }

    public FractalNoise(double noiseScale, double heightScale, int detail) {
        this(noiseScale, heightScale, detail, new Random().nextLong());
    }

    public double getHeight(double x, double y) {
        x *= noiseScale;
        y *= noiseScale;
        double height = 0.0f;
        for (int i = 0; i < sin.length; i++) {
            double freq = 1 << i;
            double s = sin[i];
            double c = cos[i];
            double dx = (x * c - y * s) * freq;
            double dy = (x * s + y * c) * freq;
            height += Noise.noise2D(dx + 10000.0d, dy + 10000.0d) * heightScale / freq;
        }
        return height;
    }

    public double getHeight(double x, double y, double detail) {
        x *= noiseScale;
        y *= noiseScale;
        double height = 0.0f;
        int id = (int) Math.floor(detail);
        if (id > 10) {
            id = 10;
            detail = 10.0d;
        }
        for (int i = 0; i < id; i++) {
            double freq = 1 << i;
            double s = sin[i];
            double c = cos[i];
            double dx = (x * c - y * s) * freq;
            double dy = (x * s + y * c) * freq;
            height += Noise.noise2D(dx + 10000.0d, dy + 10000.0d) * heightScale / freq;
        }
        double fract = detail - id;
        if (fract == 0.0d) {
            return height;
        }
        double freq = 1 << id;
        double s = sin[id];
        double c = cos[id];
        double dx = (x * c - y * s) * freq;
        double dy = (x * s + y * c) * freq;
        height += Noise.noise2D(dx + 10000.0d, dy + 10000.0d) * heightScale / freq * fract;
        return height;
    }
    
    // Old Version:
    /*public double getHeight(double x, double y) {
        x *= noiseScale;
        y *= noiseScale;
        double height = 0.0f;
        for (int i = 0; i < sin.length; i++) {
            double freq = 1 << i;
            double amp = 1.0d / freq;
            double s = sin[i];
            double c = cos[i];
            double dx = (x * c - y * s) * freq;
            double dy = (x * s + y * c) * freq;
            height += (float)Noise.noise2D(dx + 10000.0d, dy + 10000.0d) * heightScale * amp;
        }
        return height;
    }

    public double getHeight(double x, double y, double detail) {
        x *= noiseScale;
        y *= noiseScale;
        double height = 0.0f;
        int id = (int) Math.floor(detail);
        if (id > 10) {
            id = 10;
            detail = 10.0d;
        }
        for (int i = 0; i < id; i++) {
            double freq = 1 << i;
            double amp = 1.0d / freq;
            double s = sin[i];
            double c = cos[i];
            double dx = (x * c - y * s) * freq;
            double dy = (x * s + y * c) * freq;
            height += (float)Noise.noise2D(dx + 10000.0d, dy + 10000.0d) * heightScale * amp;
        }
        double fract = detail - id;
        if (fract == 0.0d) {
            return height;
        }
        double freq = 1 << id;
        double amp = 1.0d / freq;
        double s = sin[id];
        double c = cos[id];
        double dx = (x * c - y * s) * freq;
        double dy = (x * s + y * c) * freq;
        height += (float)Noise.noise2D(dx + 10000.0d, dy + 10000.0d) * heightScale *amp * fract;
        return height;
    }*/

    public Vector3f getNormal(double x, double y) {
        double h = getHeight(x, y);
        double hx = getHeight(x + EPSILON, y);
        double hy = getHeight(x, y + EPSILON);
        Vector3d tangentX = new Vector3d(EPSILON, hx - h, 0);
        Vector3d tangentY = new Vector3d(0, hy - h, EPSILON);
        Vector3d n = Vector3d.normalize(Vector3d.cross(tangentY, tangentX));
        return new Vector3f((float) n.x, (float) n.y, (float) n.z);
    }
}
