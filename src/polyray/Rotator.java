package polyray;

public class Rotator {
    
    private static final Vector3f a = new Vector3f(1.0f, 0.0f, 0.0f);
    private static final Vector3f b = new Vector3f(0.0f, 1.0f, 0.0f);
    private static final Vector3f c = new Vector3f(0.0f, 0.0f, 1.0f);
    
    public static Vector3f calcRot(Vector3f v, Vector3f ang, int prioAxis) {
        Vector3f nV = new Vector3f(0.0f, 0.0f, 0.0f);
        switch (prioAxis) {
            case 0 -> {
                nV = rotate(b, v, ang.y);
                nV = rotate(c, nV, ang.z);
                nV = rotate(a, nV, ang.x);
            }

            case 1 -> {
                nV = rotate(c, v, ang.z);
                nV = rotate(a, nV, ang.x);
                nV = rotate(b, nV, ang.y);
            }

            case 2 -> {
                nV = rotate(a, v, ang.x);
                nV = rotate(b, nV, ang.y);
                nV = rotate(c, nV, ang.z);
            }
        }
        return nV;
    }

    public static Vector3f calcInverseRot(Vector3f v, Vector3f ang, int prioAxis) {
        Vector3f nV = new Vector3f(0.0f, 0.0f, 0.0f);
        switch (prioAxis) {
            case 0 -> {
                nV = rotate(a, v, -ang.x);
                nV = rotate(c, nV, -ang.z);
                nV = rotate(b, nV, -ang.y);
            }

            case 1 -> {
                nV = rotate(b, v, -ang.y);
                nV = rotate(a, nV, -ang.x);
                nV = rotate(c, nV, -ang.z);
            }

            case 2 -> {
                nV = rotate(c, v, -ang.z);
                nV = rotate(b, nV, -ang.y);
                nV = rotate(a, nV, -ang.x);
            }
        }
        return nV;
    }

    private static Vector3f rotate(Vector3f k, Vector3f v, float ang) {
        //return Vector3f.add(Vector3f.mul(v, (float) Math.cos(ang)), Vector3f.add(Vector3f.mul(Vector3f.cross(k, v), (float) Math.sin(ang)), Vector3f.mul(k, Vector3f.dot(k, v) * (float) (1.0 - Math.cos(ang)))));
        float sinHalfAngle = (float) Math.sin(ang / 2.0);
        float cosHalfAngle = (float) Math.cos(ang / 2.0);

        float rx = k.x * sinHalfAngle;
        float ry = k.y * sinHalfAngle;
        float rz = k.z * sinHalfAngle;
        float rw = cosHalfAngle;

        float ix = rw * v.x + ry * v.z - rz * v.y;
        float iy = rw * v.y + rz * v.x - rx * v.z;
        float iz = rw * v.z + rx * v.y - ry * v.x;
        float iw = -rx * v.x - ry * v.y - rz * v.z;

        return new Vector3f(
                ix * rw + iw * -rx + iy * -rz - iz * -ry,
                iy * rw + iw * -ry + iz * -rx - ix * -rz,
                iz * rw + iw * -rz + ix * -ry - iy * -rx
        );
    }
}
