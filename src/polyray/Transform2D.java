package polyray;

import java.nio.FloatBuffer;

public class Transform2D {

    public float[] matrix = new float[9];

    public Transform2D() {
        setToIdentity();
    }

    public Transform2D(float[] matrix) {
        System.arraycopy(matrix, 0, this.matrix, 0, 9);
    }

    public void setToIdentity() {
        for (int i = 0; i < 9; i++) {
            matrix[i] = 0.0f;
        }
        matrix[0] = matrix[4] = matrix[8] = 1.0f;
    }

    public void set(Transform2D other) {
        System.arraycopy(other.matrix, 0, matrix, 0, 9);
    }

    public void setToAxisTransform(Vector2f xAxis, Vector2f yAxis, Vector2f origin) {
        matrix[0] = xAxis.x;
        matrix[1] = xAxis.y;
        matrix[2] = 0.0f;
        matrix[3] = yAxis.x;
        matrix[4] = yAxis.y;
        matrix[5] = 0.0f;
        matrix[6] = origin.x;
        matrix[7] = origin.y;
        matrix[8] = 1.0f;
    }

    public void translate(float tx, float ty) {
        Transform2D translation = new Transform2D();
        translation.matrix[6] = tx;
        translation.matrix[7] = ty;
        multiply(translation);
    }

    public void scale(float sx, float sy) {
        Transform2D scaling = new Transform2D();
        scaling.matrix[0] = sx;
        scaling.matrix[4] = sy;
        multiply(scaling);
    }

    public void rotate(float angle) {
        Transform2D rotation = new Transform2D();
        float cos = (float) Math.cos(angle);
        float sin = (float) Math.sin(angle);

        rotation.matrix[0] = cos;
        rotation.matrix[1] = -sin;
        rotation.matrix[3] = sin;
        rotation.matrix[4] = cos;

        multiply(rotation);
    }

    public void multiply(Transform2D other) {
        float[] result = new float[9];
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                result[i * 3 + j] = 0;
                for (int k = 0; k < 3; k++) {
                    result[i * 3 + j] += matrix[i * 3 + k] * other.matrix[k * 3 + j];
                }
            }
        }
        System.arraycopy(result, 0, matrix, 0, 9);
    }

    public Transform2D inverse() {
        Transform2D out = new Transform2D();
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
    
    public Transform2D transposed() {
        Transform2D out = new Transform2D(toColumnMajor());
        return out;
    }
    
    public void transpose() {
        this.matrix = toColumnMajor();
    }

    public void transform(float[] points, int nPoints) {
        for (int i = 0; i < nPoints; i++) {
            int idx = i * 2;
            float x = points[idx];
            float y = points[idx + 1];
            float newX = matrix[0] * x + matrix[3] * y + matrix[6];
            float newY = matrix[1] * x + matrix[4] * y + matrix[7];
            points[idx] = newX;
            points[idx + 1] = newY;
        }
    }

    public float[] toColumnMajor() {
        return new float[]{
            matrix[0], matrix[1], matrix[2],
            matrix[3], matrix[4], matrix[5],
            matrix[6], matrix[7], matrix[8]
        };
    }

    public FloatBuffer toFloatBuffer(FloatBuffer buffer, boolean padding) {
        if (padding) {
            buffer.put(matrix[0]).put(matrix[1]).put(matrix[2]).put(0.0f);
            buffer.put(matrix[3]).put(matrix[4]).put(matrix[5]).put(0.0f);
            buffer.put(matrix[6]).put(matrix[7]).put(matrix[8]).put(0.0f);
        } else {
            buffer.put(matrix[0]).put(matrix[1]).put(matrix[2]);
            buffer.put(matrix[3]).put(matrix[4]).put(matrix[5]);
            buffer.put(matrix[6]).put(matrix[7]).put(matrix[8]);
        }
        return buffer;
    }

    private static boolean invertMatrix(float[] m, float[] invOut) {
        float det = m[0] * m[4] * m[8]
                - m[0] * m[5] * m[7]
                - m[1] * m[3] * m[8]
                + m[1] * m[5] * m[6]
                + m[2] * m[3] * m[7]
                - m[2] * m[4] * m[6];
        if (det == 0) {
            return false;
        }
        det = 1.0f / det;

        float[] inv = new float[9];
        inv[0] = (m[4] * m[8] - m[7] * m[5]) * det;
        inv[1] = (m[2] * m[7] - m[1] * m[8]) * det;
        inv[2] = (m[1] * m[5] - m[2] * m[4]) * det;
        inv[3] = (m[5] * m[6] - m[3] * m[8]) * det;
        inv[4] = (m[0] * m[8] - m[2] * m[6]) * det;
        inv[5] = (m[2] * m[3] - m[0] * m[5]) * det;
        inv[6] = (m[3] * m[7] - m[4] * m[6]) * det;
        inv[7] = (m[1] * m[6] - m[0] * m[7]) * det;
        inv[8] = (m[0] * m[4] - m[1] * m[3]) * det;
        System.arraycopy(inv, 0, invOut, 0, 9);
        return true;
    }
}
