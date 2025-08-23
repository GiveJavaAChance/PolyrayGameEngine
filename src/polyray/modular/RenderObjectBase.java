package polyray.modular;

import static org.lwjgl.opengl.GL43.*;
import polyray.GLTexture;
import polyray.GLTexture.GLTexture2D;
import polyray.ShaderBuffer;
import polyray.ShaderProgram;
import polyray.Texture;
import polyray.VertexBufferTemplate;

public abstract class RenderObjectBase implements Renderable {

    protected boolean removed = false;

    protected ShaderProgram shader;

    protected int vao;
    protected ShaderBuffer vbo, instanceVbo;
    protected int vboLength, instanceLength;
    protected GLTexture texture;
    protected int mode = GL_TRIANGLES;

    public RenderObjectBase(ShaderProgram shader, ShaderBuffer vbo, ShaderBuffer instanceVbo, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        this.shader = shader;
        this.vbo = vbo;
        this.instanceVbo = instanceVbo;
        this.vboLength = vboTemplate.getSize();

        this.vao = glGenVertexArrays();
        glBindVertexArray(vao);
        vboTemplate.build(0, vbo.ID);
        if (instanceVboTemplate != null && instanceVbo != null) {
            this.instanceLength = instanceVboTemplate.getSize();
            instanceVboTemplate.build(vboTemplate.getAttributeCount(), instanceVbo.ID);
        }
        glBindVertexArray(0);
    }

    public RenderObjectBase(ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        this(shader, new ShaderBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW), instanceVboTemplate == null ? null : new ShaderBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW), vboTemplate, instanceVboTemplate);
    }

    public RenderObjectBase(Texture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        this(shader, vboTemplate, instanceVboTemplate);
        if (texture != null) {
            this.texture = new GLTexture2D(texture, GL_RGBA8);
        }
    }

    public RenderObjectBase(GLTexture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        this(shader, vboTemplate, instanceVboTemplate);
        this.texture = texture;
    }

    public void setRenderMode(int mode) {
        this.mode = mode;
    }

    public GLTexture getTexture() {
        return this.texture;
    }

    public ShaderProgram getShader() {
        return this.shader;
    }

    public boolean isRemoved() {
        return removed;
    }

    public void remove() {
        if (removed) {
            return;
        }
        if (vbo != null) {
            vbo.delete();
        }
        if (instanceVbo != null) {
            instanceVbo.delete();
        }
        if (vao != 0) {
            glDeleteVertexArrays(vao);
        }
        removed = true;
    }

    @Override
    public abstract void render();
}
