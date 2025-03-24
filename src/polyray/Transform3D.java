package polyray;

import java.nio.FloatBuffer;

public final class Transform3D {

    public float[] matrix = new float[16];

    public Transform3D() {
        setToIdentity();
    }

    public Transform3D(float[] matrix) {
        this.matrix = matrix;
    }

    public void setToIdentity() {
        for (int i = 0; i < 16; i++) {
            matrix[i] = 0.0f;
        }
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    }

    public void set(Transform3D other) {
        System.arraycopy(other.matrix, 0, matrix, 0, 16);
    }

    public void setToAxisTransform(Vector3f xAxis, Vector3f yAxis, Vector3f zAxis, Vector3f origin) {
        matrix[0] = xAxis.x;
        matrix[1] = xAxis.y;
        matrix[2] = xAxis.z;
        matrix[3] = 0.0f;
        matrix[4] = yAxis.x;
        matrix[5] = yAxis.y;
        matrix[6] = yAxis.z;
        matrix[7] = 0.0f;
        matrix[8] = zAxis.x;
        matrix[9] = zAxis.y;
        matrix[10] = zAxis.z;
        matrix[11] = 0.0f;
        matrix[12] = origin.x;
        matrix[13] = origin.y;
        matrix[14] = origin.z;
        matrix[15] = 1.0f;
    }

    public void translate(float tx, float ty, float tz) {
        Transform3D translation = new Transform3D();
        translation.matrix[12] = tx;
        translation.matrix[13] = ty;
        translation.matrix[14] = tz;
        multiply(translation);
    }

    public void scale(float sx, float sy, float sz) {
        Transform3D scaling = new Transform3D();
        scaling.matrix[0] = sx;
        scaling.matrix[5] = sy;
        scaling.matrix[10] = sz;
        multiply(scaling);
    }

    public void rotateX(float angle) {
        float sin = (float) Math.sin(angle);
        float cos = (float) Math.cos(angle);
        Transform3D rotation = new Transform3D();
        rotation.matrix[5] = cos;
        rotation.matrix[6] = -sin;
        rotation.matrix[9] = sin;
        rotation.matrix[10] = cos;
        multiply(rotation);
    }

    public void rotateY(float angle) {
        float sin = (float) Math.sin(angle);
        float cos = (float) Math.cos(angle);
        Transform3D rotation = new Transform3D();
        rotation.matrix[0] = cos;
        rotation.matrix[2] = sin;
        rotation.matrix[8] = -sin;
        rotation.matrix[10] = cos;
        multiply(rotation);
    }

    public void rotateZ(float angle) {
        float sin = (float) Math.sin(angle);
        float cos = (float) Math.cos(angle);
        Transform3D rotation = new Transform3D();
        rotation.matrix[0] = cos;
        rotation.matrix[1] = -sin;
        rotation.matrix[4] = sin;
        rotation.matrix[5] = cos;
        multiply(rotation);
    }

    public void multiply(Transform3D other) {
        float[] result = new float[16];
        for (int row = 0; row < 4; row++) {
            int ri = row << 2;
            for (int col = 0; col < 4; col++) {
                result[ri + col]
                        = matrix[ri] * other.matrix[col]
                        + matrix[ri + 1] * other.matrix[col + 4]
                        + matrix[ri + 2] * other.matrix[col + 8]
                        + matrix[ri + 3] * other.matrix[col + 12];
            }
        }
        System.arraycopy(result, 0, matrix, 0, 16);
    }

    public Transform3D inverse() {
        Transform3D out = new Transform3D();
        if (!invertMatrix(this.matrix, out.matrix)) {
            throw new IllegalStateException("Matrix cannot be inverted");
        }
        return out;
    }

    public void invert() {
        if (!invertMatrix(this.matrix, this.matrix)) {
            throw new IllegalStateException("Matrix cannot be inverted");
        }
    }
    
    public Transform3D transposed() {
        Transform3D out = new Transform3D(toColumnMajor());
        return out;
    }
    
    public void transpose() {
        this.matrix = toColumnMajor();
    }

    public void transform(float[] points, int nPoints) {
        for (int i = 0; i < nPoints; i++) {
            int idx = i * 3;
            float x = points[idx];
            float y = points[idx + 1];
            float z = points[idx + 2];
            float newX = matrix[0] * x + matrix[4] * y + matrix[8] * z + matrix[12];
            float newY = matrix[1] * x + matrix[5] * y + matrix[9] * z + matrix[13];
            points[idx + 2] = matrix[2] * x + matrix[6] * y + matrix[10] * z + matrix[14];
            points[idx] = newX;
            points[idx + 1] = newY;
        }
    }

    public float[] toColumnMajor() {
        return new float[]{
            matrix[0], matrix[1], matrix[2], matrix[3],
            matrix[4], matrix[5], matrix[6], matrix[7],
            matrix[8], matrix[9], matrix[10], matrix[11],
            matrix[12], matrix[13], matrix[14], matrix[15]
        };
    }

    public FloatBuffer toFloatBuffer(FloatBuffer buffer) {
        buffer.put(matrix[0]).put(matrix[1]).put(matrix[2]).put(matrix[3]);
        buffer.put(matrix[4]).put(matrix[5]).put(matrix[6]).put(matrix[7]);
        buffer.put(matrix[8]).put(matrix[9]).put(matrix[10]).put(matrix[11]);
        buffer.put(matrix[12]).put(matrix[13]).put(matrix[14]).put(matrix[15]);
        return buffer;
    }

    private static boolean invertMatrix(float[] m, float[] invOut) {
        float[] inv = new float[16];
        float det;

        inv[0] = m[5] * m[10] * m[15]
                - m[5] * m[11] * m[14]
                - m[9] * m[6] * m[15]
                + m[9] * m[7] * m[14]
                + m[13] * m[6] * m[11]
                - m[13] * m[7] * m[10];

        inv[4] = -m[4] * m[10] * m[15]
                + m[4] * m[11] * m[14]
                + m[8] * m[6] * m[15]
                - m[8] * m[7] * m[14]
                - m[12] * m[6] * m[11]
                + m[12] * m[7] * m[10];

        inv[8] = m[4] * m[9] * m[15]
                - m[4] * m[11] * m[13]
                - m[8] * m[5] * m[15]
                + m[8] * m[7] * m[13]
                + m[12] * m[5] * m[11]
                - m[12] * m[7] * m[9];

        inv[12] = -m[4] * m[9] * m[14]
                + m[4] * m[10] * m[13]
                + m[8] * m[5] * m[14]
                - m[8] * m[6] * m[13]
                - m[12] * m[5] * m[10]
                + m[12] * m[6] * m[9];

        inv[1] = -m[1] * m[10] * m[15]
                + m[1] * m[11] * m[14]
                + m[9] * m[2] * m[15]
                - m[9] * m[3] * m[14]
                - m[13] * m[2] * m[11]
                + m[13] * m[3] * m[10];

        inv[5] = m[0] * m[10] * m[15]
                - m[0] * m[11] * m[14]
                - m[8] * m[2] * m[15]
                + m[8] * m[3] * m[14]
                + m[12] * m[2] * m[11]
                - m[12] * m[3] * m[10];

        inv[9] = -m[0] * m[9] * m[15]
                + m[0] * m[11] * m[13]
                + m[8] * m[1] * m[15]
                - m[8] * m[3] * m[13]
                - m[12] * m[1] * m[11]
                + m[12] * m[3] * m[9];

        inv[13] = m[0] * m[9] * m[14]
                - m[0] * m[10] * m[13]
                - m[8] * m[1] * m[14]
                + m[8] * m[2] * m[13]
                + m[12] * m[1] * m[10]
                - m[12] * m[2] * m[9];

        inv[2] = m[1] * m[6] * m[15]
                - m[1] * m[7] * m[14]
                - m[5] * m[2] * m[15]
                + m[5] * m[3] * m[14]
                + m[13] * m[2] * m[7]
                - m[13] * m[3] * m[6];

        inv[6] = -m[0] * m[6] * m[15]
                + m[0] * m[7] * m[14]
                + m[4] * m[2] * m[15]
                - m[4] * m[3] * m[14]
                - m[12] * m[2] * m[7]
                + m[12] * m[3] * m[6];

        inv[10] = m[0] * m[5] * m[15]
                - m[0] * m[7] * m[13]
                - m[4] * m[1] * m[15]
                + m[4] * m[3] * m[13]
                + m[12] * m[1] * m[7]
                - m[12] * m[3] * m[5];

        inv[14] = -m[0] * m[5] * m[14]
                + m[0] * m[6] * m[13]
                + m[4] * m[1] * m[14]
                - m[4] * m[2] * m[13]
                - m[12] * m[1] * m[6]
                + m[12] * m[2] * m[5];

        inv[3] = -m[1] * m[6] * m[11]
                + m[1] * m[7] * m[10]
                + m[5] * m[2] * m[11]
                - m[5] * m[3] * m[10]
                - m[9] * m[2] * m[7]
                + m[9] * m[3] * m[6];

        inv[7] = m[0] * m[6] * m[11]
                - m[0] * m[7] * m[10]
                - m[4] * m[2] * m[11]
                + m[4] * m[3] * m[10]
                + m[8] * m[2] * m[7]
                - m[8] * m[3] * m[6];

        inv[11] = -m[0] * m[5] * m[11]
                + m[0] * m[7] * m[9]
                + m[4] * m[1] * m[11]
                - m[4] * m[3] * m[9]
                - m[8] * m[1] * m[7]
                + m[8] * m[3] * m[5];

        inv[15] = m[0] * m[5] * m[10]
                - m[0] * m[6] * m[9]
                - m[4] * m[1] * m[10]
                + m[4] * m[2] * m[9]
                + m[8] * m[1] * m[6]
                - m[8] * m[2] * m[5];

        det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

        if (det == 0) {
            return false;
        }

        det = 1.0f / det;
        for (int i = 0; i < 16; i++) {
            invOut[i] = inv[i] * det;
        }
        return true;
    }
}
