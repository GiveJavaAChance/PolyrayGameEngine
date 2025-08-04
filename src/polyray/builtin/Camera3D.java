package polyray.builtin;

import java.nio.FloatBuffer;
import polyray.Transform3D;
import polyray.Vector3f;
import polyray.modular.Camera;

public class Camera3D extends Camera {

    public final Transform3D cameraTransform;
    public final Vector3f pos;

    private final float[] projection, inverseProjection;
    private float nearZ, farZ;
    private float fov = (float) Math.PI * 0.5f;
    private float aspect;
    private boolean update = true;

    public Camera3D(float nearZ, float farZ) {
        this.cameraTransform = new Transform3D();
        this.pos = new Vector3f();

        this.projection = new float[16];
        this.inverseProjection = new float[16];
        projection[11] = -1.0f;
        if (nearZ < 0.1f) {
            nearZ = 0.1f;
        }
        if (farZ < 0.1f) {
            farZ = 0.1f;
        }
        this.nearZ = nearZ;
        this.farZ = farZ;
    }

    public void setFOV(float fov) {
        this.fov = fov;
        update = true;
    }

    private void updatePerspective(float aspect) {
        float yScale = (float) (1.0f / Math.tan(fov / 2.0f));
        float xScale = yScale / aspect;
        projection[0] = xScale;
        projection[5] = yScale;
        float frustumLength = farZ - nearZ;
        projection[10] = -((farZ + nearZ) / frustumLength);
        projection[14] = -((2 * nearZ * farZ) / frustumLength);
        Transform3D.invertMatrix(projection, inverseProjection);
    }

    @Override
    public void upload(int width, int height) {
        float a = (float) width / height;
        if (update || a != aspect) {
            updatePerspective(a);
            update = false;
            aspect = a;
        }
        FloatBuffer cameraData = FloatBuffer.allocate(68);
        cameraTransform.toFloatBuffer(cameraData);
        cameraTransform.inverse().toFloatBuffer(cameraData);
        cameraData.put(projection);
        cameraData.put(inverseProjection);
        cameraData.put(pos.x).put(pos.y).put(pos.z);
        cameraBuffer.uploadData(cameraData.array());
    }
}
