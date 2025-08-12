
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
import polyray.Material;
import polyray.ResourceLoader;
import polyray.ShaderPreprocessor;
import polyray.Texture;
import polyray.Transform3D;
import polyray.Vector3f;
import polyray.builtin.Camera3D;
import polyray.builtin.GenericRenderObject;
import polyray.builtin.Instance3D;
import polyray.builtin.RenderObject;
import polyray.builtin.Renderer3D;
import polyray.builtin.Vertex3D;
import polyray.objloader.OBJLoader;
import polyray.objloader.OBJLoader.RenderData;
import polyray.objloader.OBJMaterial;

public class CelShading {

    public static final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    public static final Point center = new Point(screenSize.width >> 1, screenSize.height >> 1);
    public static final Robot rob;

    public static final float CAMERA_SPEED = 4.0f;
    public static final float SPEEDUP_MUL = 2.0f;
    public static final float SLOWDOWN_MUL = 0.1f;

    public static final GLFWindow w;
    public static final Renderer3D u;

    public static final Camera3D cam;
    public static final Vector3f cameraAng = new Vector3f();

    static {
        Robot r = null;
        try {
            r = new Robot();
        } catch (AWTException e) {
        }
        rob = r;
        ResourceLoader.setResourceClass(CelShading.class);
        JFileChooser fc = new JFileChooser();
        fc.setFileFilter(new FileNameExtensionFilter("OBJ Files", "obj"));
        if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {
            throw new IllegalArgumentException("Loading canceled!");
        }
        File objFile = fc.getSelectedFile();
        fc.setFileFilter(new FileNameExtensionFilter("MTL Associate File", "mtl"));
        if (fc.showOpenDialog(null) != JFileChooser.APPROVE_OPTION) {
            throw new IllegalArgumentException("Loading canceled!");
        }
        File mtlFile = fc.getSelectedFile();

        w = new GLFWindow("Flat Shading");
        w.createFrame(500, 500, false, true, false);
        u = new Renderer3D(w.getWidth(), w.getHeight(), 16);

        cam = new Camera3D(0.1f, 1000.0f);

        Background b = new Background(cam.cameraBinding);
        u.setBackground(b);
        b.setAmbientColor(new Vector3f(0.2f, 0.1f, 0.0f));
        b.setSunColor(new Vector3f(0.8f, 0.5f, 0.3f));

        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles("Flat.vert", "Flat.frag");
        proc.appendAll();
        proc.setInt("CAM3D_IDX", cam.cameraBinding);
        proc.setInt("ENV_IDX", BindingRegistry.bindBufferBase(b.environmentBuffer));

        ArrayList<RenderData> objects = null;
        try {
            objects = OBJLoader.loadOBJFile(objFile, mtlFile);
        } catch (IOException e) {
        }
        if (objects == null) {
            throw new ExceptionInInitializerError("Loading failed!");
        }
        for (RenderData obj : objects) {
            Material mat = new Material(proc.createProgram("flat", 0));
            OBJMaterial.toPBR(obj.mat, mat);
            Texture tex;
            try {
                tex = obj.getTexture();
            } catch (IOException ex) {
                throw new ExceptionInInitializerError("Failed to load texture: " + obj.mat.mapKd);
            }
            RenderObject<Vertex3D, Instance3D> object = new RenderObject<>(tex, mat.getShader(), Vertex3D.VBO_TEMPLATE, Instance3D.VBO_TEMPLATE);
            object.vertices = obj.vertices;
            object.upload();
            u.add3DObject(object);
            object.addInstance(new Instance3D(new Transform3D()));
            object.uploadInstances();
        }
    }

    private static void freeCameraMovement(float dt) {
        Vector3f forward = new Vector3f(-cam.cameraTransform.matrix[2], -cam.cameraTransform.matrix[6], -cam.cameraTransform.matrix[10]);
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
        cam.pos.x += movement.x;
        cam.pos.y += movement.y;
        cam.pos.z += movement.z;
    }

    private static void updateTransform() {
        cam.cameraTransform.setToIdentity();
        cam.cameraTransform.rotateY(cameraAng.y);
        cam.cameraTransform.rotateX(cameraAng.x);
    }

    private static void updateCameraAngle() {
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

    public static void main(String[] args) {
        float dt = 1.0f / 165.0f;

        w.hideCursor(true);
        while (w.isWindowOpen()) {
            long startTime = System.nanoTime();

            updateCameraAngle();
            freeCameraMovement(dt);
            updateTransform();

            cam.upload(w.getWidth(), w.getHeight());
            u.render();

            w.update();
            dt = (System.nanoTime() - startTime) / 1000000000.0f;
        }
    }
}
