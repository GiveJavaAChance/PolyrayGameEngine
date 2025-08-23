package polyray;

import static org.lwjgl.opengl.GL46.*;

public abstract class GLTexture {

    public int ID;
    public final int format;

    private final int target;

    private int interpolation;
    private boolean mipmapEnabled;

    private GLTexture(int format, int target) {
        this.format = format;
        this.target = target;
        this.ID = glGenTextures();
        this.interpolation = 0;
        this.mipmapEnabled = false;
        setFiltering();
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    public void bind() {
        glBindTexture(target, ID);
    }

    public void delete() {
        glDeleteTextures(ID);
    }

    public final int getTarget() {
        return this.target;
    }

    public GLTexture setInterpolation(boolean interpolate) {
        interpolation = interpolate ? (interpolation | 1) : (interpolation & 2);
        setFiltering();
        return this;
    }

    public GLTexture setWrapMode(int mode) {
        bind();
        glTexParameteri(target, GL_TEXTURE_WRAP_S, mode);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, mode);
        return this;
    }

    public GLTexture setAnisotrophy(float level) {
        bind();
        glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY, level);
        return this;
    }

    public GLTexture enableMipmap(boolean interpolate) {
        mipmapEnabled = true;
        interpolation = interpolate ? (interpolation | 2) : (interpolation & 1);
        setFiltering();
        return this;
    }

    public void disableMipmap() {
        mipmapEnabled = false;
        setFiltering();
    }

    public void generateMipmap() {
        if (!mipmapEnabled) {
            throw new IllegalStateException("Can't generate mipmaps since mipmapping is disabled.");
        }
        bind();
        glGenerateMipmap(target);
    }

    private void setFiltering() {
        int mode = GL_NEAREST + (interpolation & 1);
        bind();
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, mipmapEnabled ? (GL_NEAREST_MIPMAP_NEAREST + interpolation) : mode);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mode);
    }

    public static final float getMaxAnisotropy() {
        return glGetFloat(GL_MAX_TEXTURE_MAX_ANISOTROPY);
    }

    public static final int getMaxTextureSize() {
        return glGetInteger(GL_MAX_TEXTURE_SIZE);
    }

    public static final int getMax3DTextureSize() {
        return glGetInteger(GL_MAX_3D_TEXTURE_SIZE);
    }

    public static final int getMaxArrayLayers() {
        return glGetInteger(GL_MAX_ARRAY_TEXTURE_LAYERS);
    }

    public static class GLTexture2D extends GLTexture {

        private final int width, height;

        public GLTexture2D(int width, int height, int format) {
            super(format, GL_TEXTURE_2D);
            this.width = width;
            this.height = height;
            bind();
            glTexStorage2D(super.target, 1, format, width, height);
        }

        public GLTexture2D(Texture texture, int format) {
            super(format, GL_TEXTURE_2D);
            this.width = texture.getWidth();
            this.height = texture.getHeight();
            bind();
            glTexImage2D(super.target, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
        }

        public void setTextureData(Texture texture, int x, int y) {
            bind();
            glTexSubImage2D(super.target, 0, x, y, texture.getWidth(), texture.getHeight(), GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
        }

        public final int getWidth() {
            return width;
        }

        public final int getHeight() {
            return height;
        }
    }

    public static class GLTextureArray extends GLTexture {

        private final int width, height, layers;

        public GLTextureArray(int width, int height, int layers, int format) {
            super(format, GL_TEXTURE_2D_ARRAY);
            this.width = width;
            this.height = height;
            this.layers = layers;
            bind();
            glTexStorage3D(super.target, 1, format, width, height, layers);
        }

        public void setLayerData(int layer, Texture texture) {
            if (layer < 0 || layer >= layers || texture.getWidth() != this.width || texture.getHeight() != this.height) {
                throw new IllegalArgumentException("Layer index out of bounds.");
            }
            bind();
            glTexSubImage3D(super.target, 0, 0, 0, layer, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, texture.getData());
        }

        public final int getWidth() {
            return width;
        }

        public final int getHeight() {
            return height;
        }

        public final int getLayers() {
            return layers;
        }
    }

    public static class GLTexture3D extends GLTexture {

        private final int width, height, depth;

        public GLTexture3D(int width, int height, int depth, int format) {
            super(format, GL_TEXTURE_3D);
            this.width = width;
            this.height = height;
            this.depth = depth;
            bind();
            glTexImage3D(GL_TEXTURE_3D, 0, format, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        }

        @Override
        public GLTexture setWrapMode(int mode) {
            bind();
            glTexParameteri(super.target, GL_TEXTURE_WRAP_S, mode);
            glTexParameteri(super.target, GL_TEXTURE_WRAP_T, mode);
            glTexParameteri(super.target, GL_TEXTURE_WRAP_R, mode);
            return this;
        }

        public void setSliceData(Texture tex, int z) {
            if (z < 0 || z >= depth) {
                throw new IllegalArgumentException("Z index out of bounds.");
            }
            bind();
            glTexSubImage3D(super.target, 0, 0, 0, z, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, tex.getData());
        }

        public final int getWidth() {
            return width;
        }

        public final int getHeight() {
            return height;
        }

        public final int getDepth() {
            return depth;
        }
    }
}
