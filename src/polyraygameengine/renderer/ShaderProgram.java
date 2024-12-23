package polyraygameengine.renderer;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import static org.lwjgl.opengl.ARBComputeShader.GL_COMPUTE_SHADER;
import static org.lwjgl.opengl.ARBGPUShaderFP64.*;
import static org.lwjgl.opengl.GL20.*;

public class ShaderProgram {

    private final int program;
    private int vs, fs, cs;
    private final boolean compute;
    private boolean isOn = false;

    private ShaderProgram(String vert, String frag, String name, int index) {
        compute = false;
        program = glCreateProgram();
        vs = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vs, vert);
        glCompileShader(vs);
        if (glGetShaderi(vs, GL_COMPILE_STATUS) != 1) {
            System.err.println(glGetShaderInfoLog(vs));
            System.exit(1);
        }
        fs = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fs, frag);
        glCompileShader(fs);
        if (glGetShaderi(fs, GL_COMPILE_STATUS) != 1) {
            System.err.println(glGetShaderInfoLog(fs));
            System.exit(1);
        }
        glAttachShader(program, vs);
        glAttachShader(program, fs);

        glBindAttribLocation(program, index, name);

        glLinkProgram(program);
        if (glGetProgrami(program, GL_LINK_STATUS) != 1) {
            System.err.println(glGetProgramInfoLog(program));
            System.exit(1);
        }
        glValidateProgram(program);
        if (glGetProgrami(program, GL_VALIDATE_STATUS) != 1) {
            System.err.println(glGetProgramInfoLog(program));
            System.exit(1);
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    private ShaderProgram(String comp, String name, int index) {
        compute = true;
        program = glCreateProgram();
        cs = glCreateShader(GL_COMPUTE_SHADER);
        glShaderSource(cs, comp);
        glCompileShader(cs);
        if (glGetShaderi(cs, GL_COMPILE_STATUS) != 1) {
            System.err.println(glGetShaderInfoLog(cs));
            System.exit(1);
        }
        glAttachShader(program, cs);

        glBindAttribLocation(program, index, name);

        glLinkProgram(program);
        if (glGetProgrami(program, GL_LINK_STATUS) != 1) {
            System.err.println(glGetProgramInfoLog(program));
            System.exit(1);
        }
        glValidateProgram(program);
        if (glGetProgrami(program, GL_VALIDATE_STATUS) != 1) {
            System.err.println(glGetProgramInfoLog(program));
            System.exit(1);
        }
        glDeleteShader(cs);
    }

    public static ShaderProgram fromFiles(String vertexFile, String fragmentFile, String name, int index) {
        ShaderProgram p = new ShaderProgram(readFile(vertexFile), readFile(fragmentFile), name, index);
        return p;
    }

    public static ShaderProgram fromFiles(String computeFile, String name, int index) {
        ShaderProgram p = new ShaderProgram(readFile(computeFile), name, index);
        return p;
    }

    public static ShaderProgram fromSource(String vertexSource, String fragmentSource, String name, int index) {
        ShaderProgram p = new ShaderProgram(vertexSource, fragmentSource, name, index);
        return p;
    }

    public static ShaderProgram fromSource(String computeSource, String name, int index) {
        ShaderProgram p = new ShaderProgram(computeSource, name, index);
        return p;
    }

    private static String readFile(String filePath) {
        StringBuilder str = new StringBuilder();
        try ( BufferedReader br = new BufferedReader(new InputStreamReader(ShaderProgram.class.getResourceAsStream(filePath)))) {
            String line;
            while ((line = br.readLine()) != null) {
                str.append(line);
                str.append("\n");
            }

        } catch (IOException e) {
        }
        return str.toString();
    }

    public void use() {
        if (!isOn) {
            isOn = true;
            glUseProgram(program);
        }
    }

    public void unuse() {
        if (isOn) {
            isOn = false;
            glUseProgram(0);
        }
    }

    public int getProgramId() {
        return program;
    }

    public boolean isComputeShader() {
        return this.compute;
    }

    public void setUniform(String name, int... v) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (v.length) {
            case 1 -> {
                glUniform1i(location, v[0]);
            }
            case 2 -> {
                glUniform2i(location, v[0], v[1]);
            }
            case 3 -> {
                glUniform3i(location, v[0], v[1], v[2]);
            }
            case 4 -> {
                glUniform4i(location, v[0], v[1], v[2], v[3]);
            }
            default -> {
                int i;
                for (i = 0; i < v.length - 4; i += 4) {
                    glUniform4i(location, v[i], v[i + 1], v[i + 2], v[i + 3]);
                }
                for (; i < v.length; i++) {
                    glUniform1i(location, v[i]);
                }
            }
        }
    }

    public void setUniform(String name, float... v) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (v.length) {
            case 1 -> {
                glUniform1f(location, v[0]);
            }
            case 2 -> {
                glUniform2f(location, v[0], v[1]);
            }
            case 3 -> {
                glUniform3f(location, v[0], v[1], v[2]);
            }
            case 4 -> {
                glUniform4f(location, v[0], v[1], v[2], v[3]);
            }
            default -> {
                int i;
                for (i = 0; i < v.length - 4; i += 4) {
                    glUniform4f(location, v[i], v[i + 1], v[i + 2], v[i + 3]);
                }
                for (; i < v.length; i++) {
                    glUniform1f(location, v[i]);
                }
            }
        }
    }

    public void setUniform(String name, double... v) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (v.length) {
            case 1 -> {
                glUniform1d(location, v[0]);
            }
            case 2 -> {
                glUniform2d(location, v[0], v[1]);
            }
            case 3 -> {
                glUniform3d(location, v[0], v[1], v[2]);
            }
            case 4 -> {
                glUniform4d(location, v[0], v[1], v[2], v[3]);
            }
            default -> {
                int i;
                for (i = 0; i < v.length - 4; i += 4) {
                    glUniform4d(location, v[i], v[i + 1], v[i + 2], v[i + 3]);
                }
                for (; i < v.length; i++) {
                    glUniform1d(location, v[i]);
                }
            }
        }
    }

    public void cleanup() {
        glDeleteProgram(program);
    }
}
