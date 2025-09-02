package polyray;

import java.util.Objects;

public class Vector4i {

    public int x;
    public int y;
    public int z;
    public int w;

    public Vector4i() {
        this.x = 0;
        this.y = 0;
        this.z = 0;
        this.w = 0;
    }

    public Vector4i(int x, int y, int z, int w) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.w = w;
    }

    public Vector4i(Vector4i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
        this.w = v.w;
    }

    public static Vector4i add(Vector4i a, Vector4i b) {
        return new Vector4i(a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w);
    }

    public static Vector4i sub(Vector4i a, Vector4i b) {
        return new Vector4i(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
    }

    public static Vector4i mul(Vector4i v, int scalar) {
        return new Vector4i(v.x * scalar, v.y * scalar, v.z * scalar, v.w * scalar);
    }

    public static Vector4i and(Vector4i v, int mask) {
        return new Vector4i(v.x & mask, v.y & mask, v.z & mask, v.w & mask);
    }

    public static Vector4i and(Vector4i a, Vector4i b) {
        return new Vector4i(a.x & b.x, a.y & b.y, a.z & b.z, a.w & b.w);
    }

    public static Vector4i or(Vector4i v, int mask) {
        return new Vector4i(v.x | mask, v.y | mask, v.z | mask, v.w | mask);
    }

    public static Vector4i or(Vector4i a, Vector4i b) {
        return new Vector4i(a.x | b.x, a.y | b.y, a.z | b.z, a.w | b.w);
    }

    public static Vector4i xor(Vector4i v, int mask) {
        return new Vector4i(v.x ^ mask, v.y ^ mask, v.z ^ mask, v.w ^ mask);
    }

    public static Vector4i xor(Vector4i a, Vector4i b) {
        return new Vector4i(a.x ^ b.x, a.y ^ b.y, a.z ^ b.z, a.w ^ b.w);
    }

    public static Vector4i not(Vector4i v) {
        return new Vector4i(~v.x, ~v.y, ~v.z, ~v.w);
    }

    public static int dot(Vector4i a, Vector4i b) {
        return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
    }

    public static Vector4i invert(Vector4i v) {
        return new Vector4i(-v.x, -v.y, -v.z, -v.w);
    }

    @Override
    public String toString() {
        return "Vector4i: (" + this.x + ", " + this.y + ", " + this.z + ", " + this.w + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector4i v) {
            return !(v.x != this.x || v.y != this.y || v.z != this.z || v.w != this.w);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y, this.z, this.w);
    }
}
