package polyray.modular;

import static org.lwjgl.opengl.GL15.GL_DYNAMIC_DRAW;
import static org.lwjgl.opengl.GL31.GL_UNIFORM_BUFFER;
import polyray.BindingRegistry;
import polyray.ShaderBuffer;

public abstract class Camera {

    public final ShaderBuffer cameraBuffer;
    public final int cameraBinding;

    public Camera() {
        this.cameraBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.cameraBinding = BindingRegistry.bindBufferBase(cameraBuffer);
    }

    public abstract void upload(int width, int height);
}
