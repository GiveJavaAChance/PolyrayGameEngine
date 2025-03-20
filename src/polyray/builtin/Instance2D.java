package polyray.builtin;

import polyray.modular.Instance;
import java.awt.geom.AffineTransform;
import java.nio.FloatBuffer;
import polyray.Transform2D;
import polyray.VertexBuffer.*;

public class Instance2D implements Instance {

    public static final VertexBufferTemplate VBO_TEMPLATE = new VertexBufferTemplate(true)
            .addAttribute(VertexAttribute.VEC3)
            .addAttribute(VertexAttribute.VEC3)
            .addAttribute(VertexAttribute.VEC3);

    public Transform2D transform;

    public Instance2D(Transform2D transform) {
        this.transform = transform;
    }

    @Override
    public void toFloatBuffer(FloatBuffer buffer) {
        this.transform.toFloatBuffer(buffer, false);
    }

    public static FloatBuffer toFloatBuffer(AffineTransform tx) {
        // Extract the AffineTransform matrix values
        double[] matrix = new double[6];
        tx.getMatrix(matrix);

        // Java2D AffineTransform matrix:
        // | m00  m01  m02 |
        // | m10  m11  m12 |
        // |  0    0    1  |
        // Invert the rotation and skew by negating the values
        FloatBuffer buffer = FloatBuffer.allocate(9);
        buffer.put((float) matrix[0]);  // m00 (no change for scale)
        buffer.put((float) -matrix[1]);  // invert m10 (rotation/skew)
        buffer.put(0.0f);         // m20 (always 0 for 2D transforms)

        buffer.put((float) -matrix[2]);  // invert m01 (rotation/skew)
        buffer.put((float) matrix[3]);   // m11 (no change for scale)
        buffer.put(0.0f);         // m21 (always 0 for 2D transforms)

        buffer.put((float) matrix[4]);  // m02 (translation x)
        buffer.put((float) -matrix[5]);  // m12 (inverted translation y)
        buffer.put(1.0f);               // m22 (always 1 for 2D transforms)
        return buffer;
    }
}
