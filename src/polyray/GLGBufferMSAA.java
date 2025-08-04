package polyray;

import static org.lwjgl.opengl.ARBFramebufferObject.GL_COLOR_ATTACHMENT0;
import static org.lwjgl.opengl.ARBFramebufferObject.GL_DEPTH_ATTACHMENT;
import static org.lwjgl.opengl.ARBFramebufferObject.GL_FRAMEBUFFER;
import static org.lwjgl.opengl.ARBFramebufferObject.GL_FRAMEBUFFER_COMPLETE;
import static org.lwjgl.opengl.ARBFramebufferObject.glBindFramebuffer;
import static org.lwjgl.opengl.ARBFramebufferObject.glCheckFramebufferStatus;
import static org.lwjgl.opengl.ARBFramebufferObject.glDeleteFramebuffers;
import static org.lwjgl.opengl.ARBFramebufferObject.glFramebufferTexture2D;
import static org.lwjgl.opengl.ARBFramebufferObject.glGenFramebuffers;
import static org.lwjgl.opengl.ARBInternalformatQuery2.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.GL20.glDrawBuffers;

public class GLGBufferMSAA {

    public final int ID;
    public final GLTextureMSAA[] textures;
    public final GLTextureMSAA depth;

    public GLGBufferMSAA(GLTextureMSAA[] textures, GLTextureMSAA depth) {
        ID = glGenFramebuffers();
        this.textures = textures;
        this.depth = depth;

        bind();

        int[] attachments = new int[textures.length];
        for (int i = 0; i < textures.length; i++) {
            GLTextureMSAA tex = textures[i];
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, tex.ID, 0);
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        if (depth != null) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth.ID, 0);
        }

        glDrawBuffers(attachments);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            throw new RuntimeException("Framebuffer not complete!");
        }

        unbind();
    }

    public GLGBufferMSAA(int width, int height, int samples, boolean doDepth, int depthFormat, int... formats) {
        ID = glGenFramebuffers();
        this.textures = new GLTextureMSAA[formats.length];
        if (doDepth) {
            this.depth = new GLTextureMSAA(width, height, depthFormat, samples);
        } else {
            this.depth = null;
        }

        bind();

        int[] attachments = new int[textures.length];
        for (int i = 0; i < formats.length; i++) {
            GLTextureMSAA tex = new GLTextureMSAA(width, height, formats[i], samples);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D_MULTISAMPLE, tex.ID, 0);
            textures[i] = tex;
            attachments[i] = GL_COLOR_ATTACHMENT0 + i;
        }

        if (doDepth) {
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE, depth.ID, 0);
        }

        glDrawBuffers(attachments);

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
