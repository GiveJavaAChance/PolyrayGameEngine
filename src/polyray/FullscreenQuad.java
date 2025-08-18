package polyray;

import static org.lwjgl.opengl.ARBVertexArrayObject.glBindVertexArray;
import static org.lwjgl.opengl.ARBVertexArrayObject.glGenVertexArrays;
import static org.lwjgl.opengl.GL20.*;

public class FullscreenQuad {

    private int quadVao, quadVbo;
    private final ShaderProgram quadProgram;
    private GLTexture texture;

    public FullscreenQuad() {
        String vertexShaderSource = "#version 330 core\n"
                + "layout(location = 0) in vec2 pos;\n"
                + "out vec2 uv;\n"
                + "void main() {\n"
                + "    uv = pos * 0.5 + 0.5;\n"
                + "    gl_Position = vec4(pos, 0.0, 1.0);\n"
                + "}\n";

        String fragmentShaderSource = "#version 330 core\n"
                + "in vec2 uv;\n"
                + "out vec4 color;\n"
                + "uniform sampler2D screenTexture;\n"
                + "void main() {\n"
                + "    color = texture(screenTexture, uv);\n"
                + "}\n";
        quadProgram = ShaderProgram.fromSource(vertexShaderSource, fragmentShaderSource, "quadshader", 0);
        setupFullscreenQuad();
    }

    private void setupFullscreenQuad() {
        float[] quadVertices = {
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, 1.0f,
            1.0f, -1.0f
        };

        quadVao = glGenVertexArrays();
        quadVbo = glGenBuffers();
        glBindVertexArray(quadVao);
        glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
        glBufferData(GL_ARRAY_BUFFER, quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 2 * Float.BYTES, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    public void setTexture(GLTexture texture) {
        this.texture = texture;
    }

    public void render() {
        quadProgram.use();
        glActiveTexture(GL_TEXTURE0);
        texture.bind();
        glBindVertexArray(quadVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        quadProgram.unuse();
    }

    public void cleanup() {
        quadProgram.cleanup();
    }
}
