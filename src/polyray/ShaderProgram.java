package polyray;

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

    static ShaderProgram fromLocalFiles(String vertexFile, String fragmentFile, String name, int index) {
        ShaderProgram p = new ShaderProgram(readLocalFile(vertexFile), readLocalFile(fragmentFile), name, index);
        return p;
    }

    static ShaderProgram fromLocalFiles(String computeFile, String name, int index) {
        ShaderProgram p = new ShaderProgram(readLocalFile(computeFile), name, index);
        return p;
    }

    private static String readFile(String filePath) {
        StringBuilder str = new StringBuilder();
        try ( BufferedReader br = new BufferedReader(new InputStreamReader(ResourceLoader.getLoader().getResourceAsStream(filePath)))) {
            String line;
            while ((line = br.readLine()) != null) {
                str.append(line);
                str.append("\n");
            }
        } catch (IOException e) {
        }
        return str.toString();
    }

    private static String readLocalFile(String filePath) {
        StringBuilder str = new StringBuilder();
        try ( BufferedReader br = new BufferedReader(new InputStreamReader(ShaderProgram.class.getResourceAsStream("shaders/" + filePath)))) {
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

    public void setUniform(String name, int... i) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (i.length) {
            case 1 -> {
                glUniform1i(location, i[0]);
            }
            case 2 -> {
                glUniform2i(location, i[0], i[1]);
            }
            case 3 -> {
                glUniform3i(location, i[0], i[1], i[2]);
            }
            case 4 -> {
                glUniform4i(location, i[0], i[1], i[2], i[3]);
            }
        }
    }

    public void setUniformArray(String name, int wrap, int... i) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (wrap) {
            case 1 -> {
                for (int j = 0; j < i.length; j++) {
                    glUniform1i(location++, i[j]);
                }
            }
            case 2 -> {
                for (int j = 0; j < i.length; j += 2) {
                    glUniform2i(location++, i[j], i[j + 1]);
                }
            }
            case 3 -> {
                for (int j = 0; j < i.length; j += 3) {
                    glUniform3i(location++, i[j], i[j + 1], i[j + 2]);
                }
            }
            case 4 -> {
                for (int j = 0; j < i.length; j += 4) {
                    glUniform4i(location++, i[j], i[j + 1], i[j + 2], i[j + 3]);
                }
            }
        }
    }

    public void setUniform(String name, float... f) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (f.length) {
            case 1 -> {
                glUniform1f(location, f[0]);
            }
            case 2 -> {
                glUniform2f(location, f[0], f[1]);
            }
            case 3 -> {
                glUniform3f(location, f[0], f[1], f[2]);
            }
            case 4 -> {
                glUniform4f(location, f[0], f[1], f[2], f[3]);
            }
        }
    }

    public void setUniformArray(String name, int wrap, float... f) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (wrap) {
            case 1 -> {
                for (int i = 0; i < f.length; i++) {
                    glUniform1f(location++, f[i]);
                }
            }
            case 2 -> {
                for (int i = 0; i < f.length; i += 2) {
                    glUniform2f(location++, f[i], f[i + 1]);
                }
            }
            case 3 -> {
                for (int i = 0; i < f.length; i += 3) {
                    glUniform3f(location++, f[i], f[i + 1], f[i + 2]);
                }
            }
            case 4 -> {
                for (int i = 0; i < f.length; i += 4) {
                    glUniform4f(location++, f[i], f[i + 1], f[i + 2], f[i + 3]);
                }
            }
        }
    }

    public void setUniform(String name, double... d) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (d.length) {
            case 1 -> {
                glUniform1d(location, d[0]);
            }
            case 2 -> {
                glUniform2d(location, d[0], d[1]);
            }
            case 3 -> {
                glUniform3d(location, d[0], d[1], d[2]);
            }
            case 4 -> {
                glUniform4d(location, d[0], d[1], d[2], d[3]);
            }
            default -> {
                int i;
                for (i = 0; i < d.length - 4; i += 4) {
                    glUniform4d(location, d[i], d[i + 1], d[i + 2], d[i + 3]);
                }
                for (; i < d.length; i++) {
                    glUniform1d(location, d[i]);
                }
            }
        }
    }

    public void setUniformArray(String name, int wrap, double... d) {
        int location = glGetUniformLocation(program, name);
        if (location == -1) {
            return;
        }
        switch (wrap) {
            case 1 -> {
                for (int i = 0; i < d.length; i++) {
                    glUniform1d(location++, d[i]);
                }
            }
            case 2 -> {
                for (int i = 0; i < d.length; i += 2) {
                    glUniform2d(location++, d[i], d[i + 1]);
                }
            }
            case 3 -> {
                for (int i = 0; i < d.length; i += 3) {
                    glUniform3d(location++, d[i], d[i + 1], d[i + 2]);
                }
            }
            case 4 -> {
                for (int i = 0; i < d.length; i += 4) {
                    glUniform4d(location++, d[i], d[i + 1], d[i + 2], d[i + 3]);
                }
            }
        }
    }

    public void cleanup() {
        glDeleteProgram(program);
    }
}
