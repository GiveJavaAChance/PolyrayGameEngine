package polyraygameengine.renderer;

import java.nio.ByteBuffer;
import static org.lwjgl.opengl.ARBShaderStorageBufferObject.GL_SHADER_STORAGE_BUFFER;
import static org.lwjgl.opengl.ARBUniformBufferObject.glBindBufferBase;
import static org.lwjgl.opengl.GL15.*;

public class SSBO {
    private final int id;

    public SSBO() {
        id = glGenBuffers();
    }

    public void bind(int bindingPoint) {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingPoint, id);
    }

    public void uploadData(int[] data) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data, GL_STATIC_DRAW);
    }
    
    public void uploadData(long[] data) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data, GL_STATIC_DRAW);
    }
    
    public void uploadData(float[] data) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data, GL_STATIC_DRAW);
    }
    
    public void uploadData(ByteBuffer data) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
        glBufferData(GL_SHADER_STORAGE_BUFFER, data, GL_STATIC_DRAW);
    }

    public void delete() {
        glDeleteBuffers(id);
    }

    public int getId() {
        return id;
    }
}
