package polyraygameengine.renderer;

import static org.lwjgl.opengl.ARBFramebufferObject.*;
import static org.lwjgl.opengl.GL11.*;

public class GLFramebuffer {

    private final int framebufferID;
    public GLTexture render;

    public GLFramebuffer(int width, int height) {
        // Generate the framebuffer and texture
        framebufferID = glGenFramebuffers();
        render = new GLTexture(width, height, GL_RGBA8, GL_RGBA, false, false);

        bind();

        // Attach the color texture to this framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render.ID, 0);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("Framebuffer not complete!");
        }

        unbind();
    }

    public final void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferID);
    }

    public final void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    public final void delete() {
        glDeleteFramebuffers(framebufferID);
        render.delete();
    }
}
