package polyray;

import static org.lwjgl.opengl.GL46.*;

public class GLGBuffer {

    public final int ID;
    public final GLTexture[] textures;
    public final GLTexture depth;

    public GLGBuffer(GLTexture[] textures, GLTexture depth) {
        ID = glGenFramebuffers();
        this.textures = textures;
        this.depth = depth;

        bind();

        int[] attachments = new int[textures.length];
        for (int i = 0; i < textures.length; i++) {
            GLTexture tex = textures[i];
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex.getTarget(), tex.ID, 0);
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        if (depth != null) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth.getTarget(), depth.ID, 0);
        }

        if (textures.length > 0) {
            glDrawBuffers(attachments);
        } else {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("Framebuffer not complete!");
        }

        unbind();
    }

    public GLGBuffer(int width, int height, boolean doDepth, int depthFormat, int... formats) {
        ID = glGenFramebuffers();
        this.textures = new GLTexture[formats.length];
        if (doDepth) {
            this.depth = new GLTexture(width, height, depthFormat, false, false);
        } else {
            this.depth = null;
        }

        bind();

        int[] attachments = new int[textures.length];
        for (int i = 0; i < formats.length; i++) {
            GLTexture tex = new GLTexture(width, height, formats[i], false, false);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, tex.getTarget(), tex.ID, 0);
            textures[i] = tex;
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        if (doDepth) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth.getTarget(), depth.ID, 0);
        }

        if (formats.length > 0) {
            glDrawBuffers(attachments);
        } else {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

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
    }
}
