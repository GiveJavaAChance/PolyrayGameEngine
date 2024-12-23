package polyraygameengine.renderer;

import java.awt.geom.AffineTransform;
import java.nio.FloatBuffer;

public class Instance {

    public static final int SIZE = 9;

    public AffineTransform transform;

    public Instance(AffineTransform transform) {
        this.transform = transform;
    }

    public void toFloatBuffer(FloatBuffer buffer) {
        // Extract the AffineTransform matrix values
        double[] matrix = new double[6];
        transform.getMatrix(matrix);

        // Java2D AffineTransform matrix:
        // | m00  m01  m02 |
        // | m10  m11  m12 |
        // |  0    0    1  |
        
        // Invert the rotation and skew by negating the values
        
        buffer.put((float) matrix[0]);  // m00 (no change for scale)
        buffer.put((float) -matrix[1]);  // invert m10 (rotation/skew)
        buffer.put(0.0f);         // m20 (always 0 for 2D transforms)

        buffer.put((float) -matrix[2]);  // invert m01 (rotation/skew)
        buffer.put((float)matrix[3]);   // m11 (no change for scale)
        buffer.put(0.0f);         // m21 (always 0 for 2D transforms)

        buffer.put((float) matrix[4]);  // m02 (translation x)
        buffer.put((float) -matrix[5]);  // m12 (inverted translation y)
        buffer.put(1.0f);               // m22 (always 1 for 2D transforms)
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
        FloatBuffer buffer = FloatBuffer.allocate(SIZE);
        buffer.put((float) matrix[0]);  // m00 (no change for scale)
        buffer.put((float) -matrix[1]);  // invert m10 (rotation/skew)
        buffer.put(0.0f);         // m20 (always 0 for 2D transforms)

        buffer.put((float) -matrix[2]);  // invert m01 (rotation/skew)
        buffer.put((float)matrix[3]);   // m11 (no change for scale)
        buffer.put(0.0f);         // m21 (always 0 for 2D transforms)

        buffer.put((float) matrix[4]);  // m02 (translation x)
        buffer.put((float) -matrix[5]);  // m12 (inverted translation y)
        buffer.put(1.0f);               // m22 (always 1 for 2D transforms)
        return buffer;
    }
}
