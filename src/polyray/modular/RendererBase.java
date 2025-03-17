package polyray.modular;

import polyray.FullscreenQuad;
import polyray.GLFramebuffer;
import polyray.GLFramebufferMSAA;
import polyray.GLTexture;
import polyray.PostProcess;
import static org.lwjgl.opengl.GL43.*;

public abstract class RendererBase {

    private GLFramebufferMSAA MSAAFrameBuffer;
    private final boolean antialiasing;

    private final GLFramebuffer frameBuffer;
    private final GLTexture rendered, output;
    private final PostProcess post;
    private final FullscreenQuad quad;

    private int targetFramebuffer = 0;

    public RendererBase(int width, int height) {
        this.antialiasing = false;
        this.frameBuffer = new GLFramebuffer(width, height);
        this.rendered = frameBuffer.render;
        this.output = new GLTexture(width, height, GL_RGBA8, GL_RGBA, true, false);
        this.post = new PostProcess(rendered, output);

        this.quad = new FullscreenQuad();
    }

    public RendererBase(int width, int height, int MSAASampleCount) {
        this.antialiasing = true;
        this.MSAAFrameBuffer = new GLFramebufferMSAA(width, height, MSAASampleCount);
        this.frameBuffer = new GLFramebuffer(width, height);
        this.rendered = frameBuffer.render;
        this.output = new GLTexture(width, height, GL_RGBA8, GL_RGBA, true, false);
        this.post = new PostProcess(rendered, output);

        this.quad = new FullscreenQuad();
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

    public GLTexture getPrePostTexture() {
        return this.rendered;
    }

    public GLTexture getRenderTexture() {
        return this.output;
    }

    public void changeEffect(int effect) {
        post.changeEffect(effect);
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

        post.runPostProcess();
        if (targetFramebuffer != -1) {
            glClear(GL_COLOR_BUFFER_BIT);
            quad.setTexture(output);
            quad.render();
        }
    }

    public void render(int width, int height) {
        start();
        renderInternal(width, height);
        end();
    }

    protected abstract void renderInternal(int width, int height);
}
