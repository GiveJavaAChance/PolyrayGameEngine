package polyraygameengine.renderer;

import static org.lwjgl.opengl.GL12.*;

public class GLTexture {

    public int ID;
    private int width, height;
    private int interFormat;
    private int interpolationMode;
    private int wrapMode;

    public GLTexture(int width, int height, int interFormat, int format, boolean interpolate, boolean wrap) {
        this.width = width;
        this.height = height;
        this.interFormat = interFormat;
        this.interpolationMode = interpolate ? GL_LINEAR : GL_NEAREST;
        this.wrapMode = wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        this.ID = glGenTextures();
        bind();

        glTexImage2D(GL_TEXTURE_2D, 0, interFormat, width, height, 0, format, GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolationMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolationMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    }

    public GLTexture(Texture texture, int interFormat, boolean interpolate, boolean wrap) {
        this.width = texture.getWidth();
        this.height = texture.getHeight();
        this.interFormat = interFormat;
        this.interpolationMode = interpolate ? GL_LINEAR : GL_NEAREST;
        this.wrapMode = wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        this.ID = glGenTextures();
        bind();

        glTexImage2D(GL_TEXTURE_2D, 0, interFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolationMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolationMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    }

    public void setTextureData(Texture texture, int x, int y) {
        if (texture.getWidth() != width || texture.getHeight() != height) {
            this.width = texture.getWidth();
            this.height = texture.getHeight();
            this.ID = glGenTextures();
            bind();

            glTexImage2D(GL_TEXTURE_2D, 0, interFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolationMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolationMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
        } else {
            bind();

            glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, texture.getWidth(), texture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
        }
    }

    public final void bind() {
        glBindTexture(GL_TEXTURE_2D, ID);
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
