package polyray.builtin;

import java.nio.FloatBuffer;
import java.util.Collection;
import static org.lwjgl.opengl.GL13.GL_TEXTURE0;
import static org.lwjgl.opengl.GL13.glActiveTexture;
import static org.lwjgl.opengl.GL30.glBindVertexArray;
import static org.lwjgl.opengl.GL31.glDrawArraysInstanced;
import polyray.GLTexture;
import polyray.ShaderBuffer;
import polyray.ShaderProgram;
import polyray.Texture;
import polyray.VertexBufferTemplate;
import polyray.modular.Instance;
import polyray.modular.RenderObjectBase;
import polyray.modular.Vertex;

public class GenericRenderObject<V extends Vertex, VS extends Collection<V>, I extends Instance, IS extends Collection<I>> extends RenderObjectBase {

    public VS vertices;
    public IS instances;
    private int numVertices = 0, numInstances = 0;

    public GenericRenderObject(ShaderProgram shader, ShaderBuffer vbo, ShaderBuffer instanceVbo, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(shader, vbo, instanceVbo, vboTemplate, instanceVboTemplate);
    }

    public GenericRenderObject(ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(shader, vboTemplate, instanceVboTemplate);
    }

    public GenericRenderObject(Texture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(texture, shader, vboTemplate, instanceVboTemplate);
    }

    public GenericRenderObject(GLTexture texture, ShaderProgram shader, VertexBufferTemplate vboTemplate, VertexBufferTemplate instanceVboTemplate) {
        super(texture, shader, vboTemplate, instanceVboTemplate);
    }
    
    public void setVertices(VS vertices) {
        this.vertices = vertices;
    }
    
    public void setInstances(IS instances) {
        this.instances = instances;
    }
    
    public void addVertex(V v) {
        this.vertices.add(v);
    }
    
    public void addInstance(I i) {
        this.instances.add(i);
    }
    
    public void removeVertex(V v) {
        this.vertices.remove(v);
    }
    
    public void removeInstance(I i) {
        this.instances.remove(i);
    }

    public void upload() {
        FloatBuffer buffer = FloatBuffer.allocate(vertices.size() * vboLength);
        for (V v : vertices) {
            v.put(buffer);
        }
        buffer.flip();

        vbo.bind();
        vbo.uploadData(buffer.array());
        this.numVertices = vertices.size();
    }

    public void uploadInstances() {
        FloatBuffer buffer = FloatBuffer.allocate(instances.size() * instanceLength);
        for (I i : instances) {
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

