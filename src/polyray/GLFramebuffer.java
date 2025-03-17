package polyray;

import static org.lwjgl.opengl.ARBFramebufferObject.*;
import static org.lwjgl.opengl.GL11.*;
import static org.lwjgl.opengl.GL14.GL_DEPTH_COMPONENT24;
import static org.lwjgl.opengl.GL14.GL_DEPTH_COMPONENT32;

public class GLFramebuffer {

    public int ID;
    public GLTexture render, depthTexture;

    public GLFramebuffer(int width, int height) {
        // Generate the framebuffer and texture
        ID = glGenFramebuffers();
        render = new GLTexture(width, height, GL_RGBA8, GL_RGBA, false, false);
        depthTexture = new GLTexture(width, height, GL_DEPTH_COMPONENT32, GL_DEPTH_COMPONENT, true, false); // Depth texture

        bind();

        // Attach the color texture to this framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render.ID, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture.ID, 0);

        // Check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("Framebuffer not complete!");
        }

        unbind();
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
