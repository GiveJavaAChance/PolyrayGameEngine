package polyraygameengine.misc;

import java.util.Objects;

public class Vector2f {

    public float x, y;

    public Vector2f() {
        this.x = 0.0f;
        this.y = 0.0f;
    }
    
    public Vector2f(float v) {
        this.x = v;
        this.y = v;
    }

    public Vector2f(float x, float y) {
        this.x = x;
        this.y = y;
    }

    public static Vector2f add(Vector2f a, Vector2f b) {
        return new Vector2f(a.x + b.x, a.y + b.y);
    }

    public static Vector2f sub(Vector2f a, Vector2f b) {
        return new Vector2f(a.x - b.x, a.y - b.y);
    }

    public static Vector2f mul(Vector2f v, float scalar) {
        return new Vector2f(v.x * scalar, v.y * scalar);
    }

    public static Vector2f div(Vector2f v, float divisor) {
        return new Vector2f(v.x / divisor, v.y / divisor);
    }

    public static Vector2f lerp(Vector2f a, Vector2f b, float f) {
        float x = a.x + f * (b.x - a.x);
        float y = a.y + f * (b.y - a.y);
        return new Vector2f(x, y);
    }

    public static float length(Vector2f v) {
        return (float) Math.sqrt(v.x * v.x + v.y * v.y);
    }

    public static float dot(Vector2f a, Vector2f b) {
        return a.x * b.x + a.y * b.y;
    }

    public static Vector2f normalize(Vector2f v) {
        float l = length(v);
        if (l == 0.0f) {
            return new Vector2f(0.0f, 0.0f);
        }
        return new Vector2f(v.x / l, v.y / l);
    }

    public static Vector2f invert(Vector2f v) {
        return new Vector2f(-v.x, -v.y);
    }

    public static Vector2f min(Vector2f a, Vector2f b) {
        return new Vector2f(Math.min(a.x, b.x), Math.min(a.y, b.y));
    }

    public static Vector2f max(Vector2f a, Vector2f b) {
        return new Vector2f(Math.max(a.x, b.x), Math.max(a.y, b.y));
    }

    public static Vector2f random() {
        return normalize(new Vector2f((float) Math.random() - 0.5f, (float) Math.random() - 0.5f));
    }

    public static float getSlope(Vector2f a, Vector2f b) {
        return (b.y - a.y) / (b.x - a.x);
    }

    @Deprecated
    /**
     * Use {@Code intersectOnLines} that is more precise and more performant
     * instead
     */
    public static Vector2f intersect(Vector2f a1, Vector2f b1, Vector2f a2, Vector2f b2) {
        boolean i1 = a1.x == b1.x;
        boolean i2 = a2.x == b2.x;
        if (i1 && i2) {
            if (a1.x == a2.x) {
                return new Vector2f(a1.x, Math.min(Math.min(a1.y, b1.y), Math.min(a2.y, b2.y)));
            } else {
                return null;
            }
        } else if (i1) {
            float slope = getSlope(a2, b2);
            float X = a1.x;
            float Y = slope * (X - a2.x) + a2.y;
            return new Vector2f(X, Y);
        } else if (i2) {
            float slope = getSlope(a1, b1);
            float X = a2.x;
            float Y = slope * (X - a1.x) + a1.y;
            return new Vector2f(X, Y);
        }
        float slope1 = getSlope(a1, b1);
        float slope2 = getSlope(a2, b2);
        float a = slope1 * a1.x;
        float b = slope2 * a2.x;
        float n = a2.y - a1.y + a + b;
        float d = slope1 - slope2;
        float X = n / d;
        float Y = slope1 * (X - a1.x) + a1.y;
        return new Vector2f(X, Y);
    }

    public static Vector2f intersectOnLines(Vector2f a1, Vector2f b1, Vector2f a2, Vector2f b2) {
        float dx1 = a1.x - b1.x;
        float dx2 = a1.x - a2.x;
        float dx3 = a2.x - b2.x;
        float dy1 = a1.y - b1.y;
        float dy2 = a1.y - a2.y;
        float dy3 = a2.y - b2.y;
        float k = dx1 * dy3 - dy1 * dx3;
        float t = (dx2 * dy3 - dy2 * dx3) / k;
        float u = (dx1 * dy2 - dy1 * dx2) / k;
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            return lerp(a1, b1, t);
        }
        return null;
    }

    @Deprecated
    /**
     * Use {@Code getPositionOnPlane} that is more accurate, precise and gives
     * negative positions as well
     */
    public static Vector2f getPositionRelativeTo(Vector2f a, Vector2f b, Vector2f pos) {
        float slope1 = getSlope(a, b);
        float slope2 = -1.0f / slope1;
        Vector2f refA = add(a, normalize(new Vector2f(1.0f, slope2)));
        Vector2f refB = add(b, normalize(new Vector2f(1.0f, slope2)));
        Vector2f refPos = add(pos, new Vector2f(1.0f, slope1));
        Vector2f i = intersect(a, refA, pos, refPos);
        Vector2f refI = intersect(b, refB, pos, refPos);
        float X = length(sub(a, i));
        float Y = length(sub(pos, i));
        float refY = length(sub(pos, refI));
        float refAB = length(sub(a, b));
        if (Y < refAB && refY > refAB) {
            Y = -Y;
        }
        float Xdist = length(sub(i, refA));
        if (Xdist > 1.0f) {
            X = -X;
        }
        return new Vector2f(X, Y);
    }

    public static Vector2f getPositionOnPlane(Vector2f xDir, Vector2f yDir, Vector2f pos) {
        float X = dot(pos, xDir);
        float Y = dot(pos, yDir);
        return new Vector2f(X, Y);
    }

    public static Vector2f getPositionOnPlane(Vector2f yDir, Vector2f pos) {
        float X = dot(pos, new Vector2f(yDir.y, -yDir.x));
        float Y = dot(pos, yDir);
        return new Vector2f(X, Y);
    }

    public static Vector2f rotate(Vector2f v, float theta) {
        float c = (float) Math.cos(theta);
        float s = (float) Math.sin(theta);
        float newX = (float) (v.x * c - v.y * s);
        float newY = (float) (v.x * s + v.y * c);
        return new Vector2f(newX, newY);
    }

    @Override
    public String toString() {
        return "Vector2f: (" + this.x + ", " + this.y + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector2f v) {
            return !(v.x != this.x || v.y != this.y);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y);
    }
}
