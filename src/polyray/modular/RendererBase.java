package polyray.modular;

import java.util.function.BiFunction;
import polyray.FullscreenQuad;
import polyray.GLFramebuffer;
import polyray.GLFramebufferMSAA;
import static org.lwjgl.opengl.GL43.*;
import polyray.GLTexture.GLTexture2D;

public abstract class RendererBase {

    private GLFramebufferMSAA MSAAFrameBuffer;
    private final boolean antialiasing;

    private final GLFramebuffer frameBuffer;
    private final GLTexture2D rendered, output;
    private final FullscreenQuad quad;

    private PostProcessor post;

    private int targetFramebuffer = 0;

    private RendererBase(int width, int height, boolean antialiasing) {
        this.antialiasing = antialiasing;
        this.frameBuffer = new GLFramebuffer(width, height);
        this.rendered = frameBuffer.render;
        this.output = new GLTexture2D(width, height, GL_RGBA8);
        this.output.setInterpolation(true);
        this.quad = new FullscreenQuad();
    }

    public RendererBase(int width, int height) {
        this(width, height, false);
    }

    public RendererBase(int width, int height, int MSAASampleCount) {
        this(width, height, true);
        this.MSAAFrameBuffer = new GLFramebufferMSAA(width, height, MSAASampleCount);
    }

    public void setRenderTarget(GLFramebuffer target) {
        this.targetFramebuffer = target.ID;
    }

    public void setToDefaultRenderTarget() {
        this.targetFramebuffer = 0;
    }

    public void setToNoRenderTarget() {
        this.targetFramebuffer = -1;
    }

    public GLTexture2D getPrePostTexture() {
        return this.rendered;
    }

    public GLTexture2D getRenderTexture() {
        return this.output;
    }
    
    public void setPostProcess(BiFunction<GLTexture2D, GLTexture2D, PostProcessor> post) {
        if(post == null) {
            this.post = null;
            return;
        }
        this.post = post.apply(rendered, output);
    }
    
    public PostProcessor getPostProcessor() {
        return this.post;
    }

    private void start() {
        if (antialiasing) {
            MSAAFrameBuffer.bind();
        } else {
            frameBuffer.bind();
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    private void end() {
        if (antialiasing) {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer.ID);
            // Ooohh, this one stinks...
            glBlitFramebuffer(0, 0, rendered.getWidth(), rendered.getHeight(), 0, 0, rendered.getWidth(), rendered.getHeight(), GL_COLOR_BUFFER_BIT, GL_NEAREST);
            MSAAFrameBuffer.unbind();
        } else {
            frameBuffer.unbind();
        }

        if (targetFramebuffer != 0 && targetFramebuffer != -1) {
            glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
        }
        if (post != null) {
            post.process();
        }
        if (targetFramebuffer != -1) {
            glClear(GL_COLOR_BUFFER_BIT);
            quad.setTexture(post == null ? rendered : output);
            quad.render();
        }
    }

    public void render() {
        start();
        renderInternal();
        end();
    }

    protected abstract void renderInternal();
}
