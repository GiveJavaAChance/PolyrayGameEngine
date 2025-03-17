package polyray;

import static org.lwjgl.opengl.ARBInternalformatQuery2.GL_TEXTURE_2D_MULTISAMPLE;
import static org.lwjgl.opengl.ARBTextureMultisample.glTexImage2DMultisample;
import static org.lwjgl.opengl.GL12.*;

public class GLTextureMSAA {

    public final int ID;
    private final int width, height;

    public GLTextureMSAA(int width, int height, int interFormat, int samples) {
        this.width = width;
        this.height = height;
        this.ID = glGenTextures();
        bind();

        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, interFormat, width, height, true);
    }

    public final void bind() {
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, ID);
    }

    public final void delete() {
        glDeleteTextures(ID);
    }

    public final int getWidth() {
        return width;
    }

    public final int getHeight() {
        return height;
    }
}