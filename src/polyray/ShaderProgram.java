package polyray;

import static org.lwjgl.opengl.GL46.*;

public class ShaderProgram {

    private final int program;
    private final boolean compute;

    private ShaderProgram(String[] sources, int... types) {
        program = glCreateProgram();
        int[] shaders = new int[sources.length];
        for (int i = 0; i < sources.length; i++) {
            int shader = compileShader(sources[i], types[i]);
            glAttachShader(program, shader);
            shaders[i] = shader;
        }
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
        for (int shader : shaders) {
            glDeleteShader(shader);
        }
        compute = sources.length == 1 && types[0] == GL_COMPUTE_SHADER;
    }

    private static int compileShader(String source, int type) {
        int shader = glCreateShader(type);
        glShaderSource(shader, source);
        glCompileShader(shader);
        if (glGetShaderi(shader, GL_COMPILE_STATUS) != 1) {
            System.err.println(glGetShaderInfoLog(shader));
            System.exit(1);
        }
        return shader;
    }

    public static ShaderProgram fromFiles(String vertexFile, String fragmentFile) {
        return fromSource(ResourceManager.getResourceAsString(vertexFile), ResourceManager.getResourceAsString(fragmentFile));
    }

    public static ShaderProgram fromFiles(String computeFile) {
        return fromSource(ResourceManager.getResourceAsString(computeFile));
    }
    
    public static ShaderProgram customFromFiles(String[] files, int[] types) {
        if(files.length != types.length) {
            throw new IllegalArgumentException("source and type count mismatch.");
        }
        String[] sources = new String[files.length];
        for (int i = 0; i < sources.length; i++) {
            sources[i] = ResourceManager.getResourceAsString(files[i]);
        }
        return new ShaderProgram(sources, types);
    }

    public static ShaderProgram fromSource(String vertexSource, String fragmentSource) {
        return new ShaderProgram(new String[]{vertexSource, fragmentSource}, GL_VERTEX_SHADER, GL_FRAGMENT_SHADER);
    }

    public static ShaderProgram fromSource(String computeSource) {
        return new ShaderProgram(new String[]{computeSource}, GL_COMPUTE_SHADER);
    }
    
    public static ShaderProgram customFromSource(String[] sources, int[] types) {
        if(sources.length != types.length) {
            throw new IllegalArgumentException("source and type count mismatch.");
        }
        return new ShaderProgram(sources, types);
    }

    public void use() {
        glUseProgram(program);
    }

    public void unuse() {
        glUseProgram(0);
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

    public void dispatchCompute(int numGroupsX, int numGroupsY, int numGroupsZ, int barriers) {
        use();
        glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
        glMemoryBarrier(barriers);
        unuse();
    }

    public void delete() {
        glDeleteProgram(program);
    }
}
