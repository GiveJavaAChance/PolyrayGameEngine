package polyray;

import static org.lwjgl.opengl.ARBFramebufferObject.*;
import static org.lwjgl.opengl.ARBInternalformatQuery2.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.GL11.GL_RGBA8;
import static org.lwjgl.opengl.GL14.GL_DEPTH_COMPONENT24;

public class GLFramebufferMSAA {

    public final int ID;
    public GLTextureMSAA render, depthTexture;

    public GLFramebufferMSAA(int width, int height, int samples) {
        // Generate the framebuffer and texture
        ID = glGenFramebuffers();
        render = new GLTextureMSAA(width, height, GL_RGBA8, samples);
        depthTexture = new GLTextureMSAA(width, height, GL_DEPTH_COMPONENT24, samples);

        bind();

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, render.ID, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depthTexture.ID, 0);

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
