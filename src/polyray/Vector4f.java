package polyray;

import java.util.Objects;

public class Vector4f {

    public float x;
    public float y;
    public float z;
    public float w;

    public Vector4f() {
        this.x = 0.0f;
        this.y = 0.0f;
        this.z = 0.0f;
        this.w = 0.0f;
    }

    public Vector4f(float x, float y, float z, float w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    public Vector4f(Vector4f v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        this.w = v.w;
    }

    public static Vector4f add(Vector4f a, Vector4f b) {
        return new Vector4f(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }

    public static Vector4f sub(Vector4f a, Vector4f b) {
        return new Vector4f(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }

    public static Vector4f mul(Vector4f v, float scalar) {
        return new Vector4f(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
    }

    public static Vector4f div(Vector4f v, float divisor) {
        return new Vector4f(v.x / divisor, v.y / divisor, v.z / divisor, v.w / divisor);
    }

    public static Vector4f lerp(Vector4f a, Vector4f b, float f) {
        float x = a.x + f * (b.x - a.x);
        float y = a.y + f * (b.y - a.y);
        float z = a.z + f * (b.z - a.z);
        float w = a.w + f * (b.w - a.w);
        return new Vector4f(x, y, z, w);
    }

    public static float length(Vector4f v) {
        return (float) Math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
    }

    public static float dot(Vector4f a, Vector4f b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    public static Vector4f normalize(Vector4f v) {
        float l = length(v);
        if (l == 0.0f) {
            return new Vector4f();
        }
        return new Vector4f(v.x / l, v.y / l, v.z / l, v.w / l);
    }

    public static Vector4f invert(Vector4f v) {
        return new Vector4f(-v.x, -v.y, -v.z, -v.w);
    }

    public static Vector4f random() {
        return Vector4f.normalize(new Vector4f((float) Math.random() - 0.5f, (float) Math.random() - 0.5f, (float) Math.random() - 0.5f, (float) Math.random() - 0.5f));
    }

    @Override
    public String toString() {
        return "Vector4f: (" + this.x + ", " + this.y + ", " + this.z + ", " + this.w + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector4f v) {
            return !(v.x != this.x || v.y != this.y || v.z != this.z || v.w != this.w);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y, this.z, this.w);
    }

}
