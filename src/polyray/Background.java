package polyray;

import static org.lwjgl.opengl.ARBVertexArrayObject.glBindVertexArray;
import static org.lwjgl.opengl.ARBVertexArrayObject.glGenVertexArrays;
import static org.lwjgl.opengl.GL43.*;

public class Background {

    private int quadVao, quadVbo;
    private final ShaderProgram quadProgram;

    public final ShaderBuffer environmentBuffer;
    public Vector3f sunDir = Vector3f.normalize(new Vector3f(1.0f, 0.2f, 0.0f));
    public Vector3f sunColor = new Vector3f(1.0f, 1.0f, 0.7f);
    public Vector3f ambientColor = new Vector3f(0.2f, 0.5f, 1.0f);

    public Background(int cameraBufferBinding) {
        String vertexShaderSource = """
                                    #version 420
                                    #append "Camera3D.glsl";
                                    
                                    layout(location = 0) in vec2 position;
                                    
                                    out vec3 rayDir;
                                    
                                    uniform float inv = 1.0;
                                    
                                    void main() {
                                        gl_Position = vec4(position, 0.0, 1.0);
                                        vec4 clipSpace = vec4(position, -1.0, 1.0);
                                        vec4 viewSpace = inverse(projection) * clipSpace;
                                        vec3 rayDirView = normalize(viewSpace.xyz / viewSpace.w);
                                        rayDir = mat3(inverse(viewMatrix)) * rayDirView * inv;
                                    }
                                    """;

        String fragmentShaderSource = """
                                      #version 420
                                      
                                      #append "Environment.glsl";
                                      
                                      in vec3 rayDir;
                                      
                                      out vec4 color;
                                      
                                      void main() {
                                          vec3 dir = normalize(rayDir);
                                          float dot = max(dot(dir, sunDir), 0.0);
                                          float thresh = 0.999;
                                          float solid = min(max(dot - thresh, 0.0) / (1.0 - thresh) * 10.0, 1.0);
                                          vec3 col = ambientColor + (pow(dot, 100.0) + solid * 5.0) * sunColor;
                                          color = vec4(col, 1.0);
                                      }
                                      """;
        this.environmentBuffer = new ShaderBuffer(GL_UNIFORM_BUFFER, GL_DYNAMIC_DRAW);
        this.environmentBuffer.uploadData(new float[]{sunDir.x, sunDir.y, sunDir.z, 0.0f, sunColor.x, sunColor.y, sunColor.z, 0.0f, ambientColor.x, ambientColor.y, ambientColor.z, 0.0f});
        ShaderPreprocessor proc = ShaderPreprocessor.fromSource(vertexShaderSource, fragmentShaderSource);
        proc.appendAll();
        proc.setInt("ENV_IDX", BindingRegistry.bindBufferBase(this.environmentBuffer));
        proc.setInt("CAM3D_IDX", cameraBufferBinding);
        quadProgram = proc.createProgram("quadshader", 0);
        setupFullscreenQuad();
    }

    public void setSunDir(Vector3f dir) {
        this.sunDir = dir;
    }

    public void setSunColor(Vector3f color) {
        this.sunColor = color;
    }

    public void setAmbientColor(Vector3f color) {
        this.ambientColor = color;
    }

    public void invertSunDir(boolean invert) {
        quadProgram.use();
        quadProgram.setUniform("inv", (invert ? -1.0f : 1.0f));
        quadProgram.unuse();
    }

    private void setupFullscreenQuad() {
        float[] quadVertices = {
            -1.0f, -1.0f,
            -1.0f, 1.0f,
            1.0f, -1.0f,
            1.0f, 1.0f
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

    public void render() {
        this.environmentBuffer.uploadData(new float[]{sunDir.x, sunDir.y, sunDir.z, 0.0f, sunColor.x, sunColor.y, sunColor.z, 0.0f, ambientColor.x, ambientColor.y, ambientColor.z, 0.0f});
        quadProgram.use();
        glBindVertexArray(quadVao);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindVertexArray(0);
        quadProgram.unuse();
    }

    public void cleanup() {
        quadProgram.cleanup();
    }
}
