package polyray.builtin;

import static org.lwjgl.opengl.ARBComputeShader.glDispatchCompute;
import static org.lwjgl.opengl.ARBShaderImageLoadStore.*;
import static org.lwjgl.opengl.GL20.*;
import polyray.BindingRegistry;
import polyray.GLTexture;
import polyray.ShaderPreprocessor;
import polyray.ShaderProgram;
import polyray.modular.PostProcessor;

public class DefaultPostProcess extends PostProcessor {

    private final int width, height;
    private final ShaderProgram computeProgram;
    private final long startTime;
    private float time;

    public DefaultPostProcess(GLTexture input, GLTexture output) {
        super(input, output);
        this.startTime = System.nanoTime();
        this.width = input.getWidth();
        this.height = input.getHeight();
        int inIdx = BindingRegistry.bindImageTexture(source, 0, false, 0, GL_READ_WRITE);
        int outIdx = BindingRegistry.bindImageTexture(target, 0, false, 0, GL_READ_WRITE);
        ShaderPreprocessor processor = ShaderPreprocessor.fromLocalFiles("post.compute");
        processor.setInt("IN_IDX", inIdx);
        processor.setInt("OUT_IDX", outIdx);
        computeProgram = processor.createProgram("offscreenrenderer", 0);
        computeProgram.use();
        computeProgram.setUniform("effect", -1);
        computeProgram.unuse();
    }

    public void changeEffect(int effect) {
        computeProgram.use();
        computeProgram.setUniform("effect", effect);
        computeProgram.unuse();
    }

    private void uploadTime() {
        time = (System.nanoTime() - startTime) / 1000000000.0f;
        computeProgram.setUniform("time", time);
    }

    public void cleanup() {
        computeProgram.cleanup();
    }

    @Override
    public void process() {
        computeProgram.use();
        uploadTime();
        glDispatchCompute((int) Math.ceil(width / 8.0), (int) Math.ceil(height / 8.0), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        computeProgram.unuse();
    }
}
