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
                                    layout(location = 0) in vec2 position;
                                    void main() {
                                        gl_Position = vec4(position, 0.0, 1.0);
                                    }
                                    """;

        String fragmentShaderSource = """
                                      #version 420
                                      #append "Environment.glsl";
                                      #append "Camera3D.glsl";
                                      
                                      uniform vec2 screenSize;
                                      
                                      float hash(float n) {
                                          return fract(sin(n) * 43758.5453123);
                                      }
                                      
                                      vec3 ditherColor(vec3 color) {
                                          // Calculate thresholds for each color channel
                                          float thresholdR = fract(color.r * 255.0);
                                          float thresholdG = fract(color.g * 255.0);
                                          float thresholdB = fract(color.b * 255.0);
                                          // Generate random values based on fragment coordinates
                                          float seedR = hash(gl_FragCoord.x + gl_FragCoord.y * 1.1763);
                                          float seedG = hash(gl_FragCoord.x + gl_FragCoord.y * 2.8765);
                                          float seedB = hash(gl_FragCoord.x + gl_FragCoord.y * 3.2906);
                                          // Compare random values to thresholds and adjust color if needed
                                          if (seedR > thresholdR) {
                                              color.r = min(color.r + 1.0 / 255.0, 1.0);
                                          }
                                          if (seedG > thresholdG) {
                                              color.g = min(color.g + 1.0 / 255.0, 1.0);
                                          }
                                          if (seedB > thresholdB) {
                                              color.b = min(color.b + 1.0 / 255.0, 1.0);
                                          }
                                          return color;
                                      }
                                      
                                      out vec4 color;
                                      
                                      void main() {
                                          vec2 ndc = (gl_FragCoord.xy / screenSize) * 2.0 - 1.0;
                                          vec4 clipSpace = vec4(ndc, -1.0, 1.0);
                                          vec4 viewSpace = inverse(projection) * clipSpace;
                                          vec3 rayDirView = normalize(viewSpace.xyz / viewSpace.w);
                                          vec3 rayDirWorld = normalize(mat3(inverse(viewMatrix)) * rayDirView);
                                          float dot = max(dot(rayDirWorld, sunDir), 0.0);
                                          float thresh = 0.999;
                                          float solid = min(max(dot - thresh, 0.0) / (1.0 - thresh) * 10.0, 1.0);
                                          vec3 col = ambientColor + (pow(dot, 100.0) + solid * 5.0) * sunColor;
                                          //col = vec3((ndc + 1.0) * 0.5, 0.0);            // Visualize NDC
                                          //col = (rayDirView + 1.0) * 0.5;          // Visualize ray in view space
                                          //col = (rayDirWorld + 1.0) * 0.5;         // Visualize ray in world space
                                          color = vec4(ditherColor(col), 1.0);
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

    private void setupFullscreenQuad() {
        float[] quadVertices = {
            // positions
            -1.0f, 1.0f,
            -1.0f, -1.0f,
            1.0f, -1.0f,
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

    public void render(float width, float height) {
        this.environmentBuffer.uploadData(new float[]{sunDir.x, sunDir.y, sunDir.z, 0.0f, sunColor.x, sunColor.y, sunColor.z, 0.0f, ambientColor.x, ambientColor.y, ambientColor.z, 0.0f});
        quadProgram.use();
        quadProgram.setUniform("screenSize", width, height);
        glBindVertexArray(quadVao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        quadProgram.unuse();
    }

    public void cleanup() {
        quadProgram.cleanup();
    }
}
