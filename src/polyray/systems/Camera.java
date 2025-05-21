package polyray.systems;

import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.Robot;
import java.awt.Toolkit;
import java.nio.FloatBuffer;
import static org.lwjgl.opengl.GL15.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;
import polyray.BindingRegistry;
import polyray.GLFWindow;
import polyray.ShaderBuffer;
import polyray.Transform3D;
import polyray.Vector3f;

public class Camera {

    private static final Robot rob = getRobot();
    private static final Dimension effectiveScreenSize = Toolkit.getDefaultToolkit().getScreenSize();
    private static final Point center = new Point(effectiveScreenSize.width >> 1, effectiveScreenSize.height >> 1);

    private static GLFWindow w;
    private static boolean inactive = false;

    public static final Vector3f pos = new Vector3f();
    public static final Vector3f ang = new Vector3f();
    public static Transform3D cameraTransform;

    public static final float[] projection = new float[16];
    private static final float[] planes = new float[6 * 4];

    private static ShaderBuffer cameraTransformBuffer;

    public static int transformBinding;

    public static float fov = (float) Math.PI * 0.5f;
    public static float minRendDist = 0.1f;

    public static final void setup(GLFWindow w, Transform3D cameraTransform) {
        Camera.w = w;
        Camera.cameraTransform = cameraTransform;
        cameraTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        transformBinding = BindingRegistry.bindBufferBase(cameraTransformBuffer);
    }

    public static final void trackMouse(boolean track) {
        inactive = !track;
    }

    public static final void update() {
        cameraTransform.setToIdentity();
        cameraTransform.translate(pos.x, pos.y, pos.z);
        cameraTransform.rotateY(-ang.y);
        cameraTransform.rotateX(-ang.x);
    }

    public static final void updateMatrix(int width, int height) {
        createReverseZPerspectiveMatrix((float) width / height);
        FloatBuffer cameraData = FloatBuffer.allocate(36);
        cameraTransform.toFloatBuffer(cameraData);
        cameraData.put(projection);
        cameraData.put(new float[]{-pos.x, -pos.y, -pos.z});
        cameraTransformBuffer.uploadData(cameraData.array());
        updateFrustumPlanes();
    }

    public static final Vector3f forward() {
        return new Vector3f(-cameraTransform.matrix[2], -cameraTransform.matrix[6], -cameraTransform.matrix[10]);
    }

    public static Vector3f right() {
        return new Vector3f(cameraTransform.matrix[0], cameraTransform.matrix[4], cameraTransform.matrix[8]);
    }

    public static Vector3f up() {
        return new Vector3f(cameraTransform.matrix[1], cameraTransform.matrix[5], cameraTransform.matrix[9]);
    }

    public static final void resetMouse() {
        rob.mouseMove(center.x, center.y);
    }

    public static final boolean isCylinderVisible(double x, double y, double z, double height, double radius) {
        float fx = -cameraTransform.matrix[2];
        float fy = -cameraTransform.matrix[6];
        float fz = -cameraTransform.matrix[10];
        double dot = (x + pos.x) * fx + (y + pos.y) * fy + (z + pos.z) * fz;
        if (dot > 0.0f) {
            return true;
        }
        if (dot + height * fy > 0.0f) {
            return true;
        }
        return dot + (fx * fx + fy * fy + fz * fz) * radius > 0.0f;
    }

    public static final boolean isAABBVisible(Vector3f min, Vector3f max) {
        Vector3f mi = Vector3f.add(min, pos);
        Vector3f ma = Vector3f.add(max, pos);
        for (int i = 0; i < 4; i++) {
            int idx = i << 2;
            float a = planes[idx];
            float b = planes[idx + 1];
            float c = planes[idx + 2];
            float x = (a >= 0.0f) ? ma.x : mi.x;
            float y = (b >= 0.0f) ? ma.y : mi.y;
            float z = (c >= 0.0f) ? ma.z : mi.z;
            if (a * x + b * y + c * z + planes[idx + 3] < 0.0f) {
                return true;
            }
        }
        return true;
    }

    public static final boolean isPointVisible(Vector3f p) {
        Vector3f po = Vector3f.add(p, pos);
        for (int i = 0; i < 4; i++) {
            int idx = i << 2;
            if (planes[idx] * po.x + planes[idx + 1] * po.y + planes[idx + 2] * po.z + planes[idx + 3] < 0.0f) {
                return true;
            }
        }
        return true;
    }

    public static final void updateCameraAngle() {
        if (inactive || !w.isWindowFocused()) {
            return;
        }
        Point m = MouseInfo.getPointerInfo().getLocation();
        float mx = (float) (m.x - center.x) / effectiveScreenSize.width;
        float my = (float) (m.y - center.y) / effectiveScreenSize.height;
        Camera.ang.x += my;
        Camera.ang.y += mx;
        rob.mouseMove(center.x, center.y);
    }

    private static Robot getRobot() {
        try {
            return new Robot();
        } catch (AWTException e) {
        }
        return null;
    }

    private static void createReverseZPerspectiveMatrix(float aspect) {
        float yScale = (float) (1.0d / Math.tan(fov * 0.5d));
        float xScale = yScale / aspect;
        projection[0] = xScale;
        projection[5] = yScale;
        projection[11] = -1.0f;
        projection[14] = minRendDist;
    }

    private static void updateFrustumPlanes() {
        Transform3D viewFrustum = new Transform3D();
        viewFrustum.multiply(cameraTransform);
        viewFrustum.multiply(new Transform3D(projection));
        float[] m = viewFrustum.matrix;
        setPlane(m[3] + m[0], m[7] + m[4], m[11] + m[8], m[15] + m[12], 0);
        setPlane(m[3] - m[0], m[7] - m[4], m[11] - m[8], m[15] - m[12], 1);
        setPlane(m[3] + m[1], m[7] + m[5], m[11] + m[9], m[15] + m[13], 2);
        setPlane(m[3] - m[1], m[7] - m[5], m[11] - m[9], m[15] - m[13], 3);
        setPlane(m[3] + m[2], m[7] + m[6], m[11] + m[10], m[15] + m[14], 4);
        setPlane(m[3] - m[2], m[7] - m[6], m[11] - m[10], m[15] - m[14], 5);
    }

    private static void setPlane(float a, float b, float c, float d, int idx) {
        float mul = 1.0f / (float) Math.sqrt(a * a + b * b + c * c);
        idx <<= 2;
        planes[idx] = a * mul;
        planes[idx + 1] = b * mul;
        planes[idx + 2] = c * mul;
        planes[idx + 3] = d * mul;
    }
}
