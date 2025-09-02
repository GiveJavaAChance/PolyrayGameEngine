package polyray;

import java.util.Objects;

public class Vector3d {

    public double x;
    public double y;
    public double z;

    public Vector3d() {
        this.x = 0.0d;
        this.y = 0.0d;
        this.z = 0.0d;
    }

    public Vector3d(double x, double y, double z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }
    
    public Vector3d(Vector3d v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
    }

    public static Vector3d add(Vector3d a, Vector3d b) {
        return new Vector3d(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    public static Vector3d sub(Vector3d a, Vector3d b) {
        return new Vector3d(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    public static Vector3d mul(Vector3d v, double scalar) {
        return new Vector3d(v.x * scalar, v.y * scalar, v.z * scalar);
    }

    public static Vector3d div(Vector3d v, double divisor) {
        return new Vector3d(v.x / divisor, v.y / divisor, v.z / divisor);
    }

    public static Vector3d lerp(Vector3d a, Vector3d b, double f) {
        double x = a.x + f * (b.x - a.x);
        double y = a.y + f * (b.y - a.y);
        double z = a.z + f * (b.z - a.z);
        return new Vector3d(x, y, z);
    }

    public static double length(Vector3d v) {
        return Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    public static Vector3d cross(Vector3d a, Vector3d b) {
        double newX = a.y * b.z - a.z * b.y;
        double newY = a.z * b.x - a.x * b.z;
        double newZ = a.x * b.y - a.y * b.x;
        return new Vector3d(newX, newY, newZ);
    }

    public static double dot(Vector3d a, Vector3d b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    public static Vector3d normalize(Vector3d v) {
        double l = length(v);
        if (l == 0.0d) {
            return new Vector3d(0.0d, 0.0d, 0.0d);
        }
        return new Vector3d(v.x / l, v.y / l, v.z / l);
    }

    public static Vector3d invert(Vector3d v) {
        return new Vector3d(-v.x, -v.y, -v.z);
    }

    public static Vector3d random() {
        return Vector3d.normalize(new Vector3d(Math.random() - 0.5d, Math.random() - 0.5d, Math.random() - 0.5d));
    }

    @Override
    public String toString() {
        return "Vector3d: (" + this.x + ", " + this.y + ", " + this.z + ")";
    }
    
    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector3d v) {
            return !(v.x != this.x || v.y != this.y || v.z != this.z);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y, this.z);
    }

}
