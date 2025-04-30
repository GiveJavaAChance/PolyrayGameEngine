package polyray.systems;

import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.Robot;
import java.awt.Toolkit;
import polyray.GLFWindow;
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

    public static final void setup(GLFWindow w, Transform3D cameraTransform) {
        Camera.w = w;
        Camera.cameraTransform = cameraTransform;
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

    public static final boolean isPointVisible(double x, double y, double z, double radius) {
        float fx = -cameraTransform.matrix[2];
        float fy = -cameraTransform.matrix[6];
        float fz = -cameraTransform.matrix[10];
        return (x + pos.x) * fx + (y + pos.y) * fy + (z + pos.z) * fz >= radius;
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
}
