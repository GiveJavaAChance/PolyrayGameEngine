
import java.awt.AWTException;
import java.awt.Dimension;
import java.awt.MouseInfo;
import java.awt.Point;
import java.awt.Robot;
import java.awt.Toolkit;
import java.util.ArrayList;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_A;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_D;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_LEFT_CONTROL;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_LEFT_SHIFT;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_S;
import static org.lwjgl.glfw.GLFW.GLFW_KEY_W;
import polyray.Background;
import polyray.BindingRegistry;
import polyray.GLFWindow;
import polyray.Input;
import polyray.Material;
import polyray.ShaderPreprocessor;
import polyray.TextureUtils;
import polyray.Transform3D;
import polyray.Vector3f;
import polyray.builtin.Camera3D;
import polyray.builtin.Instance3D;
import polyray.builtin.RenderObject;
import polyray.builtin.Renderer3D;
import polyray.builtin.SolidLoader;
import polyray.builtin.Vertex3D;

public class FreeCamera {

    public static final Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
    public static final Point center = new Point(screenSize.width >> 1, screenSize.height >> 1);
    public static final Robot rob = getRobot();

    public static final float CAMERA_SPEED = 4.0f;
    public static final float SPEEDUP_MUL = 2.0f;
    public static final float SLOWDOWN_MUL = 0.1f;

    public final GLFWindow w;
    public final Renderer3D u;

    public final Transform3D cameraTransform;
    public final Vector3f cameraPos;
    public final Vector3f cameraAng = new Vector3f();

    public FreeCamera() {
        // Create the window with a title
        w = new GLFWindow("Free Camera");

        // The width and height (500, 500) are only in effect when the window isn't fullscreen
        w.createFrame(500, 500, false, true, false);

        // MSAA 16x
        u = new Renderer3D(w.getWidth(), w.getHeight(), 16);

        // Minimum and maximium render distances
        Camera3D cam = new Camera3D(0.1f, 1000.0f);
        cameraTransform = cam.cameraTransform;
        cameraPos = cam.pos;

        Background b = new Background(cam.cameraBinding);
        u.setBackground(b);

        // Setup shader:
        int env = BindingRegistry.bindBufferBase(b.environmentBuffer);
        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles("Texture3D.vert", "Texture3D.frag");
        proc.appendAll();
        proc.setInt("CAM3D_IDX", cam.cameraBinding);
        proc.setInt("ENV_IDX", env);

        // A simple cube as an example
        Material mat = new Material(proc.createProgram());
        mat.setMetallic(0.5f);
        mat.setRoughness(0.5f);
        mat.setF0(new Vector3f(0.05f, 0.05f, 0.05f));

        // Create the cube object with a single color texture, a material, a vertex data layout and a instance data layout
        RenderObject<Vertex3D, Instance3D> cube = new RenderObject<>(TextureUtils.createColorTexture(0xFFFFFFFF), mat.getShader(), Vertex3D.VBO_TEMPLATE, Instance3D.VBO_TEMPLATE);

        // Add the mesh for a cube
        SolidLoader.addCube(cube.vertices);
        cube.upload();

        // Add the cube object to the renderer
        u.add3DObject(cube);

        // Add one instance with no transformations
        cube.addInstance(new Instance3D(new Transform3D()));
        cube.uploadInstances();

        float dt = 1.0f / 165.0f; // Initial delta time

        // Hide cursor
        w.hideCursor(true);
        while (w.isWindowOpen()) {
            long startTime = System.nanoTime();

            updateCameraAngle();
            freeCameraMovement(dt);
            updateTransform();

            cam.upload(w.getWidth(), w.getHeight());
            u.render();

            // NEVER forget to update the window, especially when it is fullscreen
            w.update();
            dt = (System.nanoTime() - startTime) / 1000000000.0f;
        }
    }

    public final void freeCameraMovement(float dt) {
        // Get forward vector from camera transform (-z axis)
        Vector3f forward = new Vector3f(-cameraTransform.matrix[2], -cameraTransform.matrix[6], -cameraTransform.matrix[10]);

        // Rotate and normalize to get the right vector for strafing
        Vector3f right = Vector3f.normalize(new Vector3f(-forward.z, 0.0f, forward.x));
        Vector3f movement = new Vector3f();

        // Accumulate total movement
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
        // Get length of movement for normalization
        float len = Vector3f.length(movement);

        // If no key is pressed, dont move
        if (len == 0.0f) {
            return;
        }

        // Speed up or slow down based on shift or control key presses
        float mul = dt * CAMERA_SPEED;
        if (Input.getKey(GLFW_KEY_LEFT_SHIFT)) {
            mul *= SPEEDUP_MUL;
        }
        if (Input.getKey(GLFW_KEY_LEFT_CONTROL)) {
            mul *= SLOWDOWN_MUL;
        }
        // Normalize movement vector and scale with speed
        mul /= len;
        movement.x *= mul;
        movement.y *= mul;
        movement.z *= mul;

        // Move the camera
        cameraPos.x += movement.x;
        cameraPos.y += movement.y;
        cameraPos.z += movement.z;
    }

    // Update the camera transform to be at the specified position and angle
    public final void updateTransform() {
        cameraTransform.setToIdentity();
        cameraTransform.rotateY(cameraAng.y);
        cameraTransform.rotateX(cameraAng.x);
    }

    // Update the camera angle based on mouse movements and recenter mouse
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

    // Util method
    private static Robot getRobot() {
        try {
            return new Robot();
        } catch (AWTException e) {
        }
        return null;
    }

    public static void main(String[] args) {
        FreeCamera p = new FreeCamera();
    }
}
