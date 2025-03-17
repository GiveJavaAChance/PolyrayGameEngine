package polyray.builtin;

import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import static org.lwjgl.opengl.GL15.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;
import polyray.BindingRegistry;
import polyray.ShaderBuffer;
import polyray.ShaderProgram;
import polyray.Transform2D;
import polyray.modular.RenderObjectBase;
import polyray.modular.RendererBase;

public class Renderer2D extends RendererBase {

    private final ArrayList<RenderObjectBase> objects = new ArrayList<>();

    private final Transform2D cameraTransform = new Transform2D();
    private final Transform2D uiTransform = new Transform2D();

    private final ShaderBuffer cameraTransformBuffer;
    private final ShaderBuffer uiTransformBuffer;

    public Renderer2D(int width, int height) {
        super(width, height);
        this.cameraTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.uiTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        BindingRegistry.bindBufferBase(this.cameraTransformBuffer);
        BindingRegistry.bindBufferBase(this.uiTransformBuffer);
    }

    public Renderer2D(int width, int height, int MSAASampleCount) {
        super(width, height, MSAASampleCount);
        this.cameraTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.uiTransformBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        BindingRegistry.bindBufferBase(this.cameraTransformBuffer);
        BindingRegistry.bindBufferBase(this.uiTransformBuffer);
    }

    public Transform2D getCameraTransform() {
        return this.cameraTransform;
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

    public void addObject(RenderObject obj) {
        this.objects.add(obj);
    }

    public void clearObjects() {
        this.objects.clear();
    }

    @Override
    protected void renderInternal(int width, int height) {
        Transform2D projection = new Transform2D();
        projection.scale(2.0f / width, 2.0f / height);
        projection.translate(-1.0f, -1.0f);
        FloatBuffer buffer = FloatBuffer.allocate(24);
        cameraTransform.toFloatBuffer(buffer, true);
        projection.toFloatBuffer(buffer, true);
        cameraTransformBuffer.uploadData(buffer.array());
        uiTransformBuffer.uploadData(uiTransform.toFloatBuffer(FloatBuffer.allocate(12), true).array());
        Iterator<RenderObjectBase> iter2d = objects.iterator();
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
