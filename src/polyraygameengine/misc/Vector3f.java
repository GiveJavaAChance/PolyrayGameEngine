package polyraygameengine.misc;

import java.util.Objects;

public class Vector3f {

    public float x, y, z;

    public Vector3f() {
        this.x = 0.0f;
        this.y = 0.0f;
        this.z = 0.0f;
    }

    public Vector3f(float v) {
        this.x = v;
        this.y = v;
        this.z = v;
    }

    public Vector3f(float x, float y, float z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public static Vector3f add(Vector3f a, Vector3f b) {
        return new Vector3f(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    public static Vector3f sub(Vector3f a, Vector3f b) {
        return new Vector3f(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    public static Vector3f mul(Vector3f v, float scalar) {
        return new Vector3f(v.x * scalar, v.y * scalar, v.z * scalar);
    }

    public static Vector3f div(Vector3f v, float divisor) {
        return new Vector3f(v.x / divisor, v.y / divisor, v.z / divisor);
    }

    public static Vector3f lerp(Vector3f a, Vector3f b, float f) {
        float x = a.x + f * (b.x - a.x);
        float y = a.y + f * (b.y - a.y);
        float z = a.z + f * (b.z - a.z);
        return new Vector3f(x, y, z);
    }

    public static float length(Vector3f v) {
        return (float) Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    public static Vector3f cross(Vector3f a, Vector3f b) {
        float newX = a.y * b.z - a.z * b.y;
        float newY = a.z * b.x - a.x * b.z;
        float newZ = a.x * b.y - a.y * b.x;
        return new Vector3f(newX, newY, newZ);
    }

    public static float dot(Vector3f a, Vector3f b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    public static Vector3f normalize(Vector3f v) {
        float l = length(v);
        if (l == 0.0f) {
            return new Vector3f(0.0f, 0.0f, 0.0f);
        }
        return new Vector3f(v.x / l, v.y / l, v.z / l);
    }

    public static Vector3f invert(Vector3f v) {
        return new Vector3f(-v.x, -v.y, -v.z);
    }

    public static Vector3f random() {
        return Vector3f.normalize(new Vector3f((float) Math.random() - 0.5f, (float) Math.random() - 0.5f, (float) Math.random() - 0.5f));
    }

    @Override
    public String toString() {
        return "Vector3f: (" + this.x + ", " + this.y + ", " + this.z + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector3f v) {
            return !(v.x != this.x || v.y != this.y || v.z == this.z);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y, this.z);
    }

}
