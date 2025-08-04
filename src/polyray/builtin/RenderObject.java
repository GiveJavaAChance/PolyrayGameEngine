package polyray.builtin;

import java.nio.FloatBuffer;
import java.util.Collection;
import polyray.GLTexture;
import polyray.ShaderProgram;
import polyray.Texture;
import polyray.VertexBufferTemplate;
import static org.lwjgl.opengl.GL43.*;
import polyray.ShaderBuffer;
import polyray.modular.Instance;
import polyray.modular.RenderObjectBase;
import polyray.modular.Vertex;

public class RenderObject extends RenderObjectBase {

    private Collection<? extends Vertex> vertices;
    private Collection<? extends Instance> instances;
    private int numVertices = 0, numInstances = 0;

    public RenderObject(ShaderProgram shader, ShaderBuffer vbo, ShaderBuffer instanceVbo, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(shader, vbo, instanceVbo, vboTemplate, instanceVboTemplate);
    }

    public RenderObject(ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(shader, vboTemplate, instanceVboTemplate);
    }

    public RenderObject(Texture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(texture, shader, vboTemplate, instanceVboTemplate);
    }

    public RenderObject(GLTexture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(texture, shader, vboTemplate, instanceVboTemplate);
    }

    public void setVertices(Collection<? extends Vertex> vertices) {
        this.vertices = vertices;
    }

    public void setInstances(Collection<? extends Instance> instances) {
        this.instances = instances;
    }

    public void upload() {
        FloatBuffer buffer = FloatBuffer.allocate(vertices.size() * vboLength);
        for (Vertex v : vertices) {
            v.put(buffer);
        }
        buffer.flip();

        vbo.bind();
        vbo.uploadData(buffer.array());
        this.numVertices = vertices.size();
    }

    public void uploadInstances() {
        FloatBuffer buffer = FloatBuffer.allocate(instances.size() * instanceLength);
        for (Instance i : instances) {
            i.toFloatBuffer(buffer);
        }
        buffer.flip();

        instanceVbo.bind();
        instanceVbo.uploadData(buffer.array());
        this.numInstances = instances.size();
    }

    public void clearCPUMemory() {
        vertices.clear();
        instances.clear();
    }

    public boolean isClear() {
        return numVertices == 0 || numInstances == 0;
    }

    @Override
    public void render() {
        shader.use();
        if (texture != null) {
            glActiveTexture(GL_TEXTURE0);
            texture.bind();
        }
        glBindVertexArray(vao);
        glDrawArraysInstanced(mode, 0, numVertices, numInstances);
        shader.unuse();
    }

}
