package polyraygameengine.renderer;

import static org.lwjgl.opengl.ARBComputeShader.glDispatchCompute;
import static org.lwjgl.opengl.ARBShaderImageLoadStore.*;
import static org.lwjgl.opengl.GL20.*;

public class PostProcess {

    private final GLTexture inputTexture;
    private final GLTexture outputTexture;
    private final int width, height;
    private ShaderProgram computeProgram;
    private FullscreenQuad quad;
    private long startTime;
    private float time;
    private int effect = -1;

    public PostProcess(GLTexture input, GLTexture output, int inIdx, int outIdx) {
        this.startTime = System.nanoTime();
        this.inputTexture = input;
        this.outputTexture = output;
        this.width = input.getWidth();
        this.height = input.getHeight();

        // Prepare the compute shader program and set up image bindings
        computeProgram = ShaderProgram.fromFiles("post.compute", "offscreenrenderer", 0);
        computeProgram.use();
        glBindImageTexture(inIdx, inputTexture.ID, 0, false, 0, GL_READ_WRITE, GL_RGBA8);
        glBindImageTexture(outIdx, outputTexture.ID, 0, false, 0, GL_READ_WRITE, GL_RGBA8);
        computeProgram.unuse();
    }

    public void changeEffect(int effect) {
        this.effect = effect;
    }

    private void uploadTime() {
        int id = computeProgram.getProgramId();
        int sizeLocation = glGetUniformLocation(id, "time");
        if (sizeLocation == -1) {
            return;
        }
        int effectLocation = glGetUniformLocation(id, "effect");
        if (effectLocation == -1) {
            return;
        }
        time = (System.nanoTime() - startTime) / 1000000000.0f;
        glUniform1f(sizeLocation, time);
        glUniform1i(effectLocation, effect);
    }

    public void runPostProcess() {
        // Use compute shader for image processing
        computeProgram.use();
        uploadTime();
        glDispatchCompute((int) Math.ceil(width / 8.0), (int) Math.ceil(height / 8.0), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        computeProgram.unuse();

    }

    public void cleanup() {
        computeProgram.cleanup();
        inputTexture.delete();
        outputTexture.delete();
    }
}
