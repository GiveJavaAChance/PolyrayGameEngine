package polyray;

import java.util.Objects;

public class Vector3i {

    public int x;
    public int y;
    public int z;

    public Vector3i() {
        this.x = 0;
        this.y = 0;
        this.z = 0;
    }

    public Vector3i(int x, int y, int z) {
        this.x = x;
        this.y = y;
        this.z = z;
    }

    public Vector3i(Vector3i v) {
        this.x = v.x;
        this.y = v.y;
        this.z = v.z;
    }

    public static Vector3i add(Vector3i a, Vector3i b) {
        return new Vector3i(a.x + b.x, a.y + b.y, a.z + b.z);
    }

    public static Vector3i sub(Vector3i a, Vector3i b) {
        return new Vector3i(a.x - b.x, a.y - b.y, a.z - b.z);
    }

    public static Vector3i mul(Vector3i v, int scalar) {
        return new Vector3i(v.x * scalar, v.y * scalar, v.z * scalar);
    }

    public static Vector3i and(Vector3i v, int mask) {
        return new Vector3i(v.x & mask, v.y & mask, v.z & mask);
    }

    public static Vector3i and(Vector3i a, Vector3i b) {
        return new Vector3i(a.x & b.x, a.y & b.y, a.z & b.z);
    }

    public static Vector3i or(Vector3i v, int mask) {
        return new Vector3i(v.x | mask, v.y | mask, v.z | mask);
    }

    public static Vector3i or(Vector3i a, Vector3i b) {
        return new Vector3i(a.x | b.x, a.y | b.y, a.z | b.z);
    }

    public static Vector3i xor(Vector3i v, int mask) {
        return new Vector3i(v.x ^ mask, v.y ^ mask, v.z ^ mask);
    }

    public static Vector3i xor(Vector3i a, Vector3i b) {
        return new Vector3i(a.x ^ b.x, a.y ^ b.y, a.z ^ b.z);
    }

    public static Vector3i not(Vector3i v) {
        return new Vector3i(~v.x, ~v.y, ~v.z);
    }

    public static int dot(Vector3i a, Vector3i b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    public static Vector3i invert(Vector3i v) {
        return new Vector3i(-v.x, -v.y, -v.z);
    }

    @Override
    public String toString() {
        return "Vector3i: (" + this.x + ", " + this.y + ", " + this.z + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector3i v) {
            return !(v.x != this.x || v.y != this.y || v.z != this.z);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y, this.z);
    }
}
