package polyraygameengine.renderer;

import java.awt.geom.AffineTransform;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import static org.lwjgl.opengl.ARBVertexArrayObject.glBindVertexArray;
import static org.lwjgl.opengl.ARBVertexArrayObject.glGenVertexArrays;
import static org.lwjgl.opengl.GL33.*;

public class Renderable {

    public ShaderProgram shader;
    private ArrayList<Vertex> vertices = new ArrayList<>();
    private ArrayList<Instance> instances = new ArrayList<>();

    private int vao;
    private int vbo;
    private int instanceVBO;
    private GLTexture texture;

    public Renderable(Texture texture, ShaderProgram shader) {
        this.shader = shader;
        this.texture = new GLTexture(texture, GL_RGBA8, false, false);
        setup();
    }

    public Renderable(GLTexture texture, ShaderProgram shader) {
        this.texture = texture;
        this.shader = shader;
        setup();
    }

    public GLTexture getTexture() {
        return this.texture;
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
        FloatBuffer buffer = FloatBuffer.allocate(vertices.size() * 4);
        for (Vertex v : vertices) {
            buffer.put(v.x).put(v.y);
            buffer.put(v.u).put(v.v);
        }
        buffer.flip();

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, buffer.array(), GL_DYNAMIC_DRAW);
    }

    public void uploadInstances() {
        FloatBuffer buffer = FloatBuffer.allocate(instances.size() * Instance.SIZE);
        for (Instance i : instances) {
            i.toFloatBuffer(buffer);
        }
        buffer.flip();

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, buffer.array(), GL_DYNAMIC_DRAW);
    }

    private void setup() {
        vao = glGenVertexArrays();
        glBindVertexArray(vao);

        vbo = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * Float.BYTES, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * Float.BYTES, 2 * Float.BYTES);
        glEnableVertexAttribArray(1);

        instanceVBO = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, Float.BYTES * instances.size() * Instance.SIZE, GL_DYNAMIC_DRAW);
        int stride = 9 * Float.BYTES;
        for (int i = 0; i < 3; i++) {
            glVertexAttribPointer(2 + i, 3, GL_FLOAT, false, stride, i * 3 * Float.BYTES);
            glEnableVertexAttribArray(2 + i);
            glVertexAttribDivisor(2 + i, 1);
        }

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    private void uploadUniforms(int width, int height, AffineTransform camera) {
        shader.setUniform("windowSize", (float) width, height);
        int id = shader.getProgramId();
        int cameraLocation = glGetUniformLocation(id, "cameraTransform");
        if (cameraLocation == -1) {
            return;
        }
        glUniformMatrix3fv(cameraLocation, false, Instance.toFloatBuffer(camera).array());
    }

    public void render(int width, int height, AffineTransform camera) {
        shader.use();
        uploadUniforms(width, height, camera);
        glActiveTexture(GL_TEXTURE0);
        texture.bind();

        glBindVertexArray(vao);
        glDrawArraysInstanced(GL_TRIANGLES, 0, vertices.size(), instances.size());

        shader.unuse();
    }
}
