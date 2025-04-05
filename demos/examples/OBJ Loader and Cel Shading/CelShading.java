import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.Robot;
import java.awt.Toolkit;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import javax.swing.JFileChooser;
import javax.swing.filechooser.FileNameExtensionFilter;
import static org.lwjgl.glfw.GLFW.*;
import polyray.Background;
import polyray.BindingRegistry;
import polyray.GLFWindow;
import polyray.Input;
import polyray.ResourceLoader;
import polyray.ShaderPreprocessor;
import polyray.Transform3D;
import polyray.Vector3f;
import polyray.builtin.Instance3D;
import polyray.builtin.RenderObject;
import polyray.builtin.Renderer3D;
import polyray.objloader.OBJLoader;

public class CelShading {

    public static final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    public static final Point center = new Point(screenSize.width >> 1, screenSize.height >> 1);
    public static final Robot rob = getRobot();

    public static final float CAMERA_SPEED = 4.0f;
    public static final float SPEEDUP_MUL = 2.0f;
    public static final float SLOWDOWN_MUL = 0.1f;

    public GLFWindow w;
    public Renderer3D u;

    public Transform3D cameraTransform;
    public final Vector3f cameraPos = new Vector3f();
    public final Vector3f cameraAng = new Vector3f();

    static {
        ResourceLoader.setResourceClass(CelShading.class);
    }

    public CelShading() {
        JFileChooser fc = new JFileChooser();
        fc.setFileFilter(new FileNameExtensionFilter("OBJ Files", "obj"));
        if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {
            return;
        }
        File objFile = fc.getSelectedFile();
        fc.setFileFilter(new FileNameExtensionFilter("MTL Associate File", "mtl"));
        if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {
            return;
        }
        File mtlFile = fc.getSelectedFile();

        w = new GLFWindow("Flat Shading");
        w.createFrame(500, 500, false, true, false);
        u = new Renderer3D(w.getWidth(), w.getHeight(), 16);
        u.setup(0.1f, 1000.0f);
        cameraTransform = u.getCameraTransform();
        
        Background b = u.getBackground();
        b.setAmbientColor(new Vector3f(0.2f, 0.1f, 0.0f));
        b.setSunColor(new Vector3f(0.8f, 0.5f, 0.3f));
        
        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles("Flat.vert", "Flat.frag");
        proc.appendAll();
        proc.setInt("CAM3D_IDX", u.getCameraTransformBinding());
        proc.setInt("ENV_IDX", BindingRegistry.bindBufferBase(b.environmentBuffer));

        ArrayList<RenderObject> objects = null;
        try {
            objects = OBJLoader.loadOBJFile(objFile, mtlFile, () -> {
                return proc.createProgram("flat", 0);
            });
        } catch (IOException e) {
        }
        if(objects == null) {
            return;
        }
        for (RenderObject obj : objects) {
            obj.upload();
            u.add3DObject(obj);
            obj.addInstance(new Instance3D(new Transform3D()));
            obj.uploadInstances();
        }

        float dt = 1.0f / 165.0f;

        w.hideCursor(true);
        while (w.isWindowOpen()) {
            long startTime = System.nanoTime();

            updateCameraAngle();
            freeCameraMovement(dt);
            updateTransform();

            u.render(w.getWidth(), w.getHeight());

            w.update();
            dt = (System.nanoTime() - startTime) / 1000000000.0f;
        }
    }

    public final void freeCameraMovement(float dt) {
        Vector3f forward = new Vector3f(-cameraTransform.matrix[2], -cameraTransform.matrix[6], -cameraTransform.matrix[10]);
        Vector3f right = Vector3f.normalize(new Vector3f(-forward.z, 0.0f, forward.x));
        Vector3f movement = new Vector3f();
        if (Input.getKey(GLFW_KEY_W)) {
            movement.x += forward.x;
            movement.y += forward.y;
            movement.z += forward.z;
        }
        if (Input.getKey(GLFW_KEY_S)) {
            movement.x -= forward.x;
            movement.y -= forward.y;
            movement.z -= forward.z;
        }
        if (Input.getKey(GLFW_KEY_D)) {
            movement.x += right.x;
            movement.y += right.y;
            movement.z += right.z;
        }
        if (Input.getKey(GLFW_KEY_A)) {
            movement.x -= right.x;
            movement.y -= right.y;
            movement.z -= right.z;
        }
        float len = Vector3f.length(movement);
        if (len == 0.0f) {
            return;
        }
        float mul = dt * CAMERA_SPEED;
        if (Input.getKey(GLFW_KEY_LEFT_SHIFT)) {
            mul *= SPEEDUP_MUL;
        }
        if (Input.getKey(GLFW_KEY_LEFT_CONTROL)) {
            mul *= SLOWDOWN_MUL;
        }
        mul /= len;
        movement.x *= mul;
        movement.y *= mul;
        movement.z *= mul;
        cameraPos.x += movement.x;
        cameraPos.y += movement.y;
        cameraPos.z += movement.z;
    }

    public final void updateTransform() {
        cameraTransform.setToIdentity();
        cameraTransform.translate(-cameraPos.x, -cameraPos.y, -cameraPos.z);
        cameraTransform.rotateY(cameraAng.y);
        cameraTransform.rotateX(cameraAng.x);
    }

    private void updateCameraAngle() {
        if (!w.isWindowFocused()) {
            return;
        }
        Point mouse = MouseInfo.getPointerInfo().getLocation();
        float mx = (float) (mouse.x - center.x) / screenSize.width;
        float my = (float) (mouse.y - center.y) / screenSize.height;
        rob.mouseMove(center.x, center.y);
        cameraAng.x -= my;
        cameraAng.y -= mx;
    }

    private static Robot getRobot() {
        try {
            return new Robot();
        } catch (AWTException e) {
        }
        return null;
    }

    public static void main(String[] args) {
        CelShading p = new CelShading();
    }

}
