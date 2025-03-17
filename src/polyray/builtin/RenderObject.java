package polyray.builtin;

import java.util.ArrayList;
import polyray.GLTexture;
import polyray.ShaderProgram;
import polyray.Texture;
import polyray.VertexBuffer;
import static org.lwjgl.opengl.GL43.*;
import polyray.VertexBuffer.VertexBufferTemplate;
import polyray.modular.Instance;
import polyray.modular.RenderObjectBase;
import polyray.modular.Vertex;

public class RenderObject extends RenderObjectBase {

    public RenderObject(ShaderProgram shader, VertexBuffer vbo, VertexBuffer instanceVbo) {
        super(shader, vbo, instanceVbo);
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

    public void setVertices(ArrayList<Vertex> vertices) {
        this.vertices = vertices;
    }

    public void setInstances(ArrayList<Instance> instances) {
        this.instances = instances;
    }

    public void addAllVertices(ArrayList<Vertex> vertices) {
        this.vertices.addAll(vertices);
    }

    public void addAllInstances(ArrayList<Instance> instances) {
        this.instances.addAll(instances);
    }

    @Override
    public void render() {
        if (texture != null) {
            glActiveTexture(GL_TEXTURE0);
            texture.bind();
        }

        glBindVertexArray(vao);
        glDrawArraysInstanced(mode, 0, numVertices, numInstances);
    }

}
