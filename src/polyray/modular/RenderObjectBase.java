package polyray.modular;

import java.nio.FloatBuffer;
import java.util.ArrayList;
import static org.lwjgl.opengl.GL43.*;
import polyray.GLTexture;
import polyray.ShaderProgram;
import polyray.Texture;
import polyray.VertexBuffer;
import polyray.VertexBuffer.VertexBufferTemplate;

public abstract class RenderObjectBase {

    public boolean doRender = true;
    protected boolean removed = false;

    protected ShaderProgram shader;
    protected ArrayList<Vertex> vertices = new ArrayList<>();
    protected ArrayList<Instance> instances = new ArrayList<>();
    protected int numVertices = 0, numInstances = 0;

    protected int vao;
    protected VertexBuffer vbo, instanceVbo;
    protected GLTexture texture;
    protected int mode = GL_TRIANGLES;

    public RenderObjectBase(ShaderProgram shader, VertexBuffer vbo, VertexBuffer instanceVbo) {
        this.shader = shader;
        this.vbo = vbo;
        this.instanceVbo = instanceVbo;
    }

    public RenderObjectBase(ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        this.shader = shader;
        vao = glGenVertexArrays();
        glBindVertexArray(vao);
        vbo = vboTemplate.build(0);
        if (instanceVboTemplate != null) {
            instanceVbo = instanceVboTemplate.build(vbo.numAttributes);
        }
        glBindVertexArray(0);
    }

    public RenderObjectBase(Texture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        this(shader, vboTemplate, instanceVboTemplate);
        this.texture = new GLTexture(texture, GL_RGBA8, false, false);
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

    public void addVertex(Vertex v) {
        vertices.add(v);
    }

    public void addTriangle(Vertex a, Vertex b, Vertex c) {
        vertices.add(a);
        vertices.add(b);
        vertices.add(c);
    }

    public void addInstance(Instance i) {
        instances.add(i);
    }

    public void upload() {
        FloatBuffer buffer = FloatBuffer.allocate(vertices.size() * vbo.length);
        for (Vertex v : vertices) {
            v.put(buffer);
        }
        buffer.flip();

        vbo.bind();
        vbo.setData(buffer.array(), true);
        this.numVertices = vertices.size();
    }

    public void uploadInstances() {
        FloatBuffer buffer = FloatBuffer.allocate(instances.size() * instanceVbo.length);
        for (Instance i : instances) {
            i.toFloatBuffer(buffer);
        }
        buffer.flip();

        instanceVbo.bind();
        instanceVbo.setData(buffer.array(), true);
        this.numInstances = instances.size();
    }

    public void clearCPUMemory() {
        vertices.clear();
        instances.clear();
    }

    public boolean isRemoved() {
        return removed;
    }

    public boolean isClear() {
        return numVertices == 0 || numInstances == 0;
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

    public abstract void render();
}
