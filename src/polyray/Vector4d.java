package polyray;

import java.util.Objects;

public class Vector4d {

    public double x;
    public double y;
    public double z;
    public double w;

    public Vector4d() {
        this.x = 0.0d;
        this.y = 0.0d;
        this.z = 0.0d;
        this.w = 0.0d;
    }

    public Vector4d(double x, double y, double z, double w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    public Vector4d(Vector4d v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        this.w = v.w;
    }

    public static Vector4d add(Vector4d a, Vector4d b) {
        return new Vector4d(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }

    public static Vector4d sub(Vector4d a, Vector4d b) {
        return new Vector4d(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }

    public static Vector4d mul(Vector4d v, double scalar) {
        return new Vector4d(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
    }

    public static Vector4d div(Vector4d v, double divisor) {
        return new Vector4d(v.x / divisor, v.y / divisor, v.z / divisor, v.w / divisor);
    }

    public static Vector4d lerp(Vector4d a, Vector4d b, double f) {
        double x = a.x + f * (b.x - a.x);
        double y = a.y + f * (b.y - a.y);
        double z = a.z + f * (b.z - a.z);
        double w = a.w + f * (b.w - a.w);
        return new Vector4d(x, y, z, w);
    }

    public static double length(Vector4d v) {
        return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    }

    public static double dot(Vector4d a, Vector4d b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    public static Vector4d normalize(Vector4d v) {
        double l = length(v);
        if (l == 0.0d) {
            return new Vector4d();
        }
        return new Vector4d(v.x / l, v.y / l, v.z / l, v.w / l);
    }

    public static Vector4d invert(Vector4d v) {
        return new Vector4d(-v.x, -v.y, -v.z, -v.w);
    }

    public static Vector4d random() {
        return Vector4d.normalize(new Vector4d(Math.random() - 0.5d, Math.random() - 0.5d, Math.random() - 0.5d, Math.random() - 0.5d));
    }

    @Override
    public String toString() {
        return "Vector4d: (" + this.x + ", " + this.y + ", " + this.z + ", " + this.w + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector4d v) {
            return !(v.x != this.x || v.y != this.y || v.z != this.z || v.w != this.w);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y, this.z, this.w);
    }

}
