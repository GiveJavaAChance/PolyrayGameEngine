package polyray;

import static org.lwjgl.opengl.ARBFramebufferObject.*;
import static org.lwjgl.opengl.GL14.*;
import polyray.GLTexture.GLTexture2D;

public class GLFramebuffer {

    public final int ID;
    public final GLTexture2D render, depthTexture;

    public GLFramebuffer(int width, int height, int colorFormat, int depthFormat) {
        ID = glGenFramebuffers();
        render = new GLTexture2D(width, height, colorFormat);
        depthTexture = new GLTexture2D(width, height, depthFormat);
        bind();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, render.getTarget(), render.ID, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture.getTarget(), depthTexture.ID, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("Framebuffer not complete!");
        }
        unbind();
    }

    public GLFramebuffer(int width, int height) {
        this(width, height, GL_RGBA8, GL_DEPTH_COMPONENT32);
    }

    public final void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, ID);
    }

    public final void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    public final void delete() {
        glDeleteFramebuffers(ID);
        render.delete();
    }
}
