package polyraygameengine.renderer;

import static org.lwjgl.opengl.GL11.*;

public class GLTexture {
    public int ID;
    private int width, height;
    private int internalFormat;
    
    public GLTexture(int width, int height, int interFormat, int format) {
        this.width = width;
        this.height = height;
        this.internalFormat = interFormat;
        this.ID = glGenTextures();
        bind();

        glTexImage2D(GL_TEXTURE_2D, 0, interFormat, width, height, 0, format, GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    public GLTexture(Texture texture, int interFormat) {
        this.width = texture.getWidth();
        this.height = texture.getHeight();
        this.internalFormat = interFormat;
        this.ID = glGenTextures();
        bind();

        glTexImage2D(GL_TEXTURE_2D, 0, interFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    
    public void setTextureData(Texture texture) {
        if (texture.getWidth() != width || texture.getHeight() != height) {
            this.width = texture.getWidth();
            this.height = texture.getHeight();
            this.ID = glGenTextures();
            bind();

            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        } else {
            bind();

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
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
