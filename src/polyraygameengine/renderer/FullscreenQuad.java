package polyraygameengine.renderer;

import static org.lwjgl.opengl.ARBVertexArrayObject.glBindVertexArray;
import static org.lwjgl.opengl.ARBVertexArrayObject.glGenVertexArrays;
import static org.lwjgl.opengl.GL20.*;

public class FullscreenQuad {
    private int quadVao, quadVbo;
    private final ShaderProgram quadProgram;
    
    public FullscreenQuad() {
        String vertexShaderSource = "#version 330 core\n"
                + "layout(location = 0) in vec2 position;\n"
                + "layout(location = 1) in vec2 texCoords;\n"
                + "out vec2 v_TexCoords;\n"
                + "void main() {\n"
                + "    v_TexCoords = texCoords;\n"
                + "    gl_Position = vec4(position, 0.0, 1.0);\n"
                + "}\n";

        String fragmentShaderSource = "#version 330 core\n"
                + "in vec2 v_TexCoords;\n"
                + "out vec4 color;\n"
                + "uniform sampler2D screenTexture;\n"
                + "void main() {\n"
                + "    color = texture(screenTexture, v_TexCoords);\n"
                + "}\n";
        quadProgram = ShaderProgram.fromSource(vertexShaderSource, fragmentShaderSource, "quadshader", 0);
        setupFullscreenQuad();
    }
    private void setupFullscreenQuad() {
        float[] quadVertices = {
            // positions  // texCoords
            -1.0f, 1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f
        };

        quadVao = glGenVertexArrays();
        quadVbo = glGenBuffers();
        glBindVertexArray(quadVao);
        glBindBuffer(GL_ARRAY_BUFFER, quadVbo);
        glBufferData(GL_ARRAY_BUFFER, quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * Float.BYTES, 0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * Float.BYTES, 2 * Float.BYTES);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    
    
    public void setTexture(GLTexture texture) {
        quadProgram.use();
        texture.bind();
        quadProgram.unuse();
    }
    
    public void render() {
        quadProgram.use();
        glBindVertexArray(quadVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        quadProgram.unuse();
    }
    
    public void cleanup() {
        quadProgram.cleanup();
    }
}
