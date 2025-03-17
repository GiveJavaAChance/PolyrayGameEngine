package polyray;

import static org.lwjgl.opengl.GL43.*;

public class GLTexture {

    public int ID;
    protected int width, height;
    protected int interFormat;
    protected int interpolationMode;
    protected int wrapMode;

    private GLTexture() {
    }

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

    public void setTextureData(Texture texture) {
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

            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.getWidth(), texture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
        }
    }

    public void setTextureData(Texture texture, int x, int y) {
        bind();
        glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, texture.getWidth(), texture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
    }

    public void resize(int newWidth, int newHeight) {
        int fbo = glGenFramebuffers();
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ID, 0);

        int newTextureID = glGenTextures();
        glBindTexture(GL_TEXTURE_2D, newTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, interFormat, newWidth, newHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, newTextureID);

        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, Math.min(width, newWidth), Math.min(height, newHeight));

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(fbo);
        glDeleteTextures(ID);

        this.ID = newTextureID;
        this.width = newWidth;
        this.height = newHeight;

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, interpolationMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, interpolationMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
    }

    public void bind() {
        glBindTexture(GL_TEXTURE_2D, ID);
    }

    public void delete() {
        glDeleteTextures(ID);
    }

    public final int getWidth() {
        return width;
    }

    public final int getHeight() {
        return height;
    }

    public static class GLTextureArray extends GLTexture {

        private final int layers;

        public GLTextureArray(int width, int height, int layers, int interFormat, boolean interpolate, boolean wrap) {
            this.width = width;
            this.height = height;
            this.layers = layers;
            this.interFormat = interFormat;
            this.interpolationMode = interpolate ? GL_LINEAR : GL_NEAREST;
            this.wrapMode = wrap ? GL_REPEAT : GL_CLAMP_TO_EDGE;
            this.ID = glGenTextures();

            glBindTexture(GL_TEXTURE_2D_ARRAY, ID);

            glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, interFormat, width, height, layers);

            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, interpolationMode);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, interpolationMode);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, wrapMode);
            glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, wrapMode);

            glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
        }

        @Override
        public void setTextureData(Texture texture) {

        }

        @Override
        public void setTextureData(Texture texture, int x, int y) {

        }

        @Override
        public void resize(int newWidth, int newHeight) {

        }

        public void setLayerData(int layer, Texture texture) {
            if (layer < 0 || layer >= layers) {
                throw new IllegalArgumentException("Layer index out of bounds.");
            }
            glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
        }

        @Override
        public void bind() {
            glBindTexture(GL_TEXTURE_2D_ARRAY, ID);
        }
    }
}
