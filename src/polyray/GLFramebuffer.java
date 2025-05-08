package polyray;

import static org.lwjgl.opengl.ARBFramebufferObject.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL14.GL_DEPTH_COMPONENT32;

public class GLFramebuffer {

    public int ID;
    public GLTexture render, depthTexture;

    public GLFramebuffer(int width, int height, int colorFormat, int colorInterFormat, int depthFormat, int depthInterFormat) {
        ID = glGenFramebuffers();

        render = new GLTexture(width, height, colorFormat, colorInterFormat, false, false);
        depthTexture = new GLTexture(width, height, depthFormat, depthInterFormat, true, false);

        bind();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render.ID, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.ID, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("Framebuffer not complete!");
        }

        unbind();
    }

    public GLFramebuffer(int width, int height) {
        this(width, height, GL_RGBA8, GL_RGBA, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT);
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
