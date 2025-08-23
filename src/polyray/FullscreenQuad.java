package polyray;

import static org.lwjgl.opengl.ARBVertexArrayObject.glBindVertexArray;
import static org.lwjgl.opengl.ARBVertexArrayObject.glGenVertexArrays;
import static org.lwjgl.opengl.GL20.*;
import polyray.GLTexture.GLTexture2D;

public class FullscreenQuad {

    private int quadVao, quadVbo;
    private final ShaderProgram quadProgram;
    private GLTexture2D texture;

    public FullscreenQuad(String fragmentShaderSource) {
        String vertexShaderSource = """
                                    #version 330 core
                                    layout(location = 0) in vec2 pos;
                                    out vec2 uv;
                                    void main() {
                                        uv = pos * 0.5 + 0.5;
                                        gl_Position = vec4(pos, 0.0, 1.0);
                                    }
                                    """;
        quadProgram = ShaderProgram.fromSource(vertexShaderSource, fragmentShaderSource, "quadshader", 0);
        setupFullscreenQuad();
    }

    public FullscreenQuad() {
        this("""
             #version 330 core
             in vec2 uv;
             out vec4 color;
             uniform sampler2D screenTexture;
             void main() {
                 color = texture(screenTexture, uv);
             }
             """);
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

    public void setTexture(GLTexture2D texture) {
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
