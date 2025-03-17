package polyray;

import java.util.Objects;

public class Vector2d {

    public double x;
    public double y;

    public Vector2d() {
        this.x = 0.0d;
        this.y = 0.0d;
    }

    public Vector2d(double x, double y) {
        this.x = x;
        this.y = y;
    }
    
    public Vector2d(Vector2d v) {
        this.x = v.x;
        this.y = v.y;
    }

    public static Vector2d add(Vector2d a, Vector2d b) {
        return new Vector2d(a.x + b.x, a.y + b.y);
    }

    public static Vector2d sub(Vector2d a, Vector2d b) {
        return new Vector2d(a.x - b.x, a.y - b.y);
    }

    public static Vector2d mul(Vector2d v, double scalar) {
        return new Vector2d(v.x * scalar, v.y * scalar);
    }

    public static Vector2d div(Vector2d v, double divisor) {
        return new Vector2d(v.x / divisor, v.y / divisor);
    }

    public static Vector2d lerp(Vector2d a, Vector2d b, double f) {
        double x = a.x + f * (b.x - a.x);
        double y = a.y + f * (b.y - a.y);
        return new Vector2d(x, y);
    }

    public static double length(Vector2d v) {
        return Math.sqrt(v.x * v.x + v.y * v.y);
    }

    public static double dot(Vector2d a, Vector2d b) {
        return a.x * b.x + a.y * b.y;
    }

    public static Vector2d normalize(Vector2d v) {
        double l = length(v);
        if (l == 0.0d) {
            return new Vector2d(0.0d, 0.0d);
        }
        return new Vector2d(v.x / l, v.y / l);
    }

    public static Vector2d invert(Vector2d v) {
        return new Vector2d(-v.x, -v.y);
    }

    public static Vector2d min(Vector2d a, Vector2d b) {
        return new Vector2d(Math.min(a.x, b.x), Math.min(a.y, b.y));
    }

    public static Vector2d max(Vector2d a, Vector2d b) {
        return new Vector2d(Math.max(a.x, b.x), Math.max(a.y, b.y));
    }

    public static Vector2d random() {
        return Vector2d.normalize(new Vector2d(Math.random() - 0.5d, Math.random() - 0.5d));
    }

    public static double getSlope(Vector2d a, Vector2d b) {
        return (b.y - a.y) / (b.x - a.x);
    }

    @Deprecated
    /**
     * Use {@Code intersectOnLines} that is more reliable and more performant
     * instead
     */
    public static Vector2d intersect(Vector2d a1, Vector2d b1, Vector2d a2, Vector2d b2) {
        boolean i1 = a1.x == b1.x;
        boolean i2 = a2.x == b2.x;
        if (i1 && i2) {
            if (a1.x == a2.x) {
                return new Vector2d(a1.x, Math.min(Math.min(a1.y, b1.y), Math.min(a2.y, b2.y)));
            } else {
                return null;
            }
        } else if (i1) {
            double slope = getSlope(a2, b2);
            double X = a1.x;
            double Y = slope * (X - a2.x) + a2.y;
            return new Vector2d(X, Y);
        } else if (i2) {
            double slope = getSlope(a1, b1);
            double X = a2.x;
            double Y = slope * (X - a1.x) + a1.y;
            return new Vector2d(X, Y);
        }
        double slope1 = getSlope(a1, b1);
        double slope2 = getSlope(a2, b2);
        double a = slope1 * a1.x;
        double b = slope2 * a2.x;
        double n = a2.y - a1.y + a + b;
        double d = slope1 - slope2;
        double X = n / d;
        double Y = slope1 * (X - a1.x) + a1.y;
        return new Vector2d(X, Y);
    }

    public static Vector2d intersectOnLines(Vector2d a1, Vector2d b1, Vector2d a2, Vector2d b2) {
        double dx1 = a1.x - b1.x;
        double dx2 = a1.x - a2.x;
        double dx3 = a2.x - b2.x;
        double dy1 = a1.y - b1.y;
        double dy2 = a1.y - a2.y;
        double dy3 = a2.y - b2.y;
        double k = dx1 * dy3 - dy1 * dx3;
        double t = (dx2 * dy3 - dy2 * dx3) / k;
        double u = (dx1 * dy2 - dy1 * dx2) / k;
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            return Vector2d.lerp(a1, b1, t);
        }
        return null;
    }

    @Deprecated
    /**
     * Use {@Code getPositionOnPlane} that is more accurate, precise and gives
     * negative positions as well
     */
    public static Vector2d getPositionRelativeTo(Vector2d a, Vector2d b, Vector2d pos) {
        double slope1 = getSlope(a, b);
        double slope2 = -1.0f / slope1;
        Vector2d refA = add(a, normalize(new Vector2d(1.0f, slope2)));
        Vector2d refB = add(b, normalize(new Vector2d(1.0f, slope2)));
        Vector2d refPos = add(pos, new Vector2d(1.0f, slope1));
        Vector2d i = intersect(a, refA, pos, refPos);
        Vector2d refI = intersect(b, refB, pos, refPos);
        double X = length(sub(a, i));
        double Y = length(sub(pos, i));
        double refY = length(sub(pos, refI));
        double refAB = length(sub(a, b));
        if (Y < refAB && refY > refAB) {
            Y = -Y;
        }
        double Xdist = length(sub(i, refA));
        if (Xdist > 1.0f) {
            X = -X;
        }
        return new Vector2d(X, Y);
    }

    public static Vector2d getPositionOnPlane(Vector2d xDir, Vector2d yDir, Vector2d pos) {
        double X = dot(pos, xDir);
        double Y = dot(pos, yDir);
        return new Vector2d(X, Y);
    }

    public static Vector2d getPositionOnPlane(Vector2d yDir, Vector2d pos) {
        double X = dot(pos, new Vector2d(yDir.y, -yDir.x));
        double Y = dot(pos, yDir);
        return new Vector2d(X, Y);
    }

    public static Vector2d rotate(Vector2d v, double theta) {
        double c = (double) Math.cos(theta);
        double s = (double) Math.sin(theta);
        double newX = (double) (v.x * c - v.y * s);
        double newY = (double) (v.x * s + v.y * c);
        return new Vector2d(newX, newY);
    }

    @Override
    public String toString() {
        return "Vector2d: (" + this.x + ", " + this.y + ")";
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) {
            return true;
        }
        if (o == null) {
            return false;
        }
        if (o instanceof Vector2d v) {
            return !(v.x != this.x || v.y != this.y);
        }
        return false;
    }

    @Override
    public int hashCode() {
        return Objects.hash(this.x, this.y);
    }
}
