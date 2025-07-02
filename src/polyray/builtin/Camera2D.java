package polyray.builtin;

import java.nio.FloatBuffer;
import polyray.Transform2D;
import polyray.modular.Camera;

public class Camera2D extends Camera {

    public final Transform2D cameraTransform;

    private final Transform2D projection, inverseProjection;
    private int prevWidth, prevHeight;

    public Camera2D() {
        this.cameraTransform = new Transform2D();
        this.projection = new Transform2D();
        this.inverseProjection = new Transform2D();
    }

    @Override
    public void upload(int width, int height) {
        if (prevWidth != width || prevHeight != height) {
            projection.setToIdentity();
            projection.scale(2.0f / width, -2.0f / height);
            projection.translate(-1.0f, 1.0f);
            Transform2D.invertMatrix(projection.matrix, inverseProjection.matrix);
            prevWidth = width;
            prevHeight = height;
        }
        FloatBuffer cameraData = FloatBuffer.allocate(48);
        cameraTransform.toFloatBuffer(cameraData, true);
        cameraTransform.inverse().toFloatBuffer(cameraData, true);
        projection.toFloatBuffer(cameraData, true);
        inverseProjection.toFloatBuffer(cameraData, true);
        cameraBuffer.uploadData(cameraData.array());
    }

}
