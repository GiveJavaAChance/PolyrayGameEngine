package polyray;

import java.util.Objects;

public class Vector2i {

    public int x;
    public int y;

    public Vector2i() {
        this.x = 0;
        this.y = 0;
    }

    public Vector2i(int x, int y) {
        this.x = x;
        this.y = y;
    }

    public Vector2i(Vector2i v) {
        this.x = v.x;
        this.y = v.y;
    }

    public static Vector2i add(Vector2i a, Vector2i b) {
        return new Vector2i(a.x + b.x, a.y + b.y);
    }

    public static Vector2i sub(Vector2i a, Vector2i b) {
        return new Vector2i(a.x - b.x, a.y - b.y);
    }

    public static Vector2i mul(Vector2i v, int scalar) {
        return new Vector2i(v.x * scalar, v.y * scalar);
    }
    
    public static Vector2i and(Vector2i v, int mask) {
        return new Vector2i(v.x & mask, v.y & mask);
    }
    
    public static Vector2i and(Vector2i a, Vector2i b) {
        return new Vector2i(a.x & b.x, a.y & b.y);
    }
    
    public static Vector2i or(Vector2i v, int mask) {
        return new Vector2i(v.x | mask, v.y | mask);
    }
    
    public static Vector2i or(Vector2i a, Vector2i b) {
        return new Vector2i(a.x | b.x, a.y | b.y);
    }
    
    public static Vector2i xor(Vector2i v, int mask) {
        return new Vector2i(v.x ^ mask, v.y ^ mask);
    }
    
    public static Vector2i xor(Vector2i a, Vector2i b) {
        return new Vector2i(a.x ^ b.x, a.y ^ b.y);
    }
    
    public static Vector2i not(Vector2i v) {
        return new Vector2i(~v.x, ~v.y);
    }

    public static int dot(Vector2i a, Vector2i b) {
        return a.x * b.x + a.y * b.y;
    }

    public static Vector2i invert(Vector2i v) {
        return new Vector2i(-v.x, -v.y);
    }

    public static Vector2i min(Vector2i a, Vector2i b) {
        return new Vector2i(Math.min(a.x, b.x), Math.min(a.y, b.y));
    }

    public static Vector2i max(Vector2i a, Vector2i b) {
        return new Vector2i(Math.max(a.x, b.x), Math.max(a.y, b.y));
    }

    @Override
    public String toString() {
        return "Vector2i: (" + this.x + ", " + this.y + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector2i v) {
            return !(v.x != this.x || v.y != this.y);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y);
    }
}
