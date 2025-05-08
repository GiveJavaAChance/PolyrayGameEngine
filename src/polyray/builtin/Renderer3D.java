package polyray.builtin;

import polyray.modular.RendererBase;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import polyray.Transform3D;
import polyray.Background;
import polyray.BindingRegistry;
import polyray.ShaderBuffer;
import polyray.ShaderProgram;
import static org.lwjgl.opengl.GL43.*;
import polyray.Transform2D;
import polyray.modular.RenderObjectBase;

public class Renderer3D extends RendererBase {

    private final ArrayList<RenderObjectBase> objects2D = new ArrayList<>();
    private final ArrayList<RenderObjectBase> objects3D = new ArrayList<>();

    public float minRendDist = 1.0f, renderDist = 1000.0f;
    private final FloatBuffer projection = FloatBuffer.allocate(16);
    private float FOV = (float) Math.PI * 0.5f;

    private boolean showWireframe = false;

    private final Transform3D cameraTransform = new Transform3D();
    private final Transform2D uiTransform = new Transform2D();

    private final ShaderBuffer cameraTransformBuffer;
    private final ShaderBuffer uiTransformBuffer;

    private final Background background;

    public Renderer3D(int width, int height) {
        super(width, height);
        this.cameraTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.uiTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.background = new Background(BindingRegistry.bindBufferBase(this.cameraTransformBuffer));
        BindingRegistry.bindBufferBase(this.uiTransformBuffer);
    }

    public Renderer3D(int width, int height, int MSAASampleCount) {
        super(width, height, MSAASampleCount);
        this.cameraTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.uiTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.background = new Background(BindingRegistry.bindBufferBase(this.cameraTransformBuffer));
        BindingRegistry.bindBufferBase(this.uiTransformBuffer);
    }

    public Transform3D getCameraTransform() {
        return cameraTransform;
    }

    public Transform2D getUITransform() {
        return this.uiTransform;
    }

    public int getCameraTransformBinding() {
        return BindingRegistry.bindBufferBase(this.cameraTransformBuffer);
    }

    public int getUITransformBinding() {
        return BindingRegistry.bindBufferBase(this.uiTransformBuffer);
    }

    public Background getBackground() {
        return this.background;
    }

    public void setup(float minRendDist, float renderDist) {
        if (minRendDist < 0.1f) {
            minRendDist = 0.1f;
        }
        if (renderDist < 0.1f) {
            renderDist = 0.1f;
        }
        this.minRendDist = minRendDist;
        this.renderDist = renderDist;
    }

    public void setFOV(float fov) {
        this.FOV = fov;
    }

    private static FloatBuffer createPerspectiveMatrix(float fov, float aspect, float zNear, float zFar, FloatBuffer matrix) {
        float yScale = (float) (1.0f / Math.tan(fov / 2.0f));
        float xScale = yScale / aspect;
        float frustumLength = zFar - zNear;
        matrix.put(0, xScale);
        matrix.put(1, 0);
        matrix.put(2, 0);
        matrix.put(3, 0);

        matrix.put(4, 0);
        matrix.put(5, yScale);
        matrix.put(6, 0);
        matrix.put(7, 0);

        matrix.put(8, 0);
        matrix.put(9, 0);
        matrix.put(10, -((zFar + zNear) / frustumLength));
        matrix.put(11, -1);

        matrix.put(12, 0);
        matrix.put(13, 0);
        matrix.put(14, -((2 * zNear * zFar) / frustumLength));
        matrix.put(15, 0);
        return matrix;
    }

    public void toggleWireframe() {
        this.showWireframe = !this.showWireframe;
    }

    public void add2DObject(RenderObjectBase r) {
        objects2D.add(r);
    }

    public void add3DObject(RenderObjectBase r) {
        objects3D.add(r);
    }
    
    public void remove2DObject(RenderObjectBase r) {
        objects2D.remove(r);
    }

    public void remove3DObject(RenderObjectBase r) {
        objects3D.remove(r);
    }

    public void clearObjects() {
        objects2D.clear();
        objects3D.clear();
    }

    @Override
    protected void renderInternal(int width, int height) {
        createPerspectiveMatrix(FOV, (float) width / height, minRendDist, renderDist, projection);
        FloatBuffer cameraData = FloatBuffer.allocate(36);
        cameraTransform.toFloatBuffer(cameraData);
        cameraData.put(projection.array());
        cameraData.put(cameraTransform.inverse().matrix, 12, 3);
        cameraTransformBuffer.uploadData(cameraData.array());
        uiTransformBuffer.uploadData(uiTransform.toFloatBuffer(FloatBuffer.allocate(12), true).array());
        background.render();

        glEnable(GL_DEPTH_TEST);

        if (showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        Iterator<RenderObjectBase> iter3d = objects3D.iterator();
        while (iter3d.hasNext()) {
            RenderObjectBase object = iter3d.next();
            if (object.isRemoved()) {
                iter3d.remove();
                continue;
            }
            if (object.isClear() || !object.doRender) {
                continue;
            }
            ShaderProgram shader = object.getShader();
            shader.use();
            object.render();
            shader.unuse();
        }

        if (showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glDisable(GL_DEPTH_TEST);

        Iterator<RenderObjectBase> iter2d = objects2D.iterator();
        while (iter2d.hasNext()) {
            RenderObjectBase object = iter2d.next();
            if (object.isRemoved()) {
                iter2d.remove();
                continue;
            }
            if (object.isClear() || !object.doRender) {
                continue;
            }
            ShaderProgram shader = object.getShader();
            shader.use();
            object.render();
            shader.unuse();
        }
    }
}
