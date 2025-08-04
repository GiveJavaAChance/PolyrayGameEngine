package polyray;

import java.nio.ByteBuffer;
import static org.lwjgl.opengl.GL46.*;

public class ShaderBuffer {
    
    public final int ID;
    public final int target;
    private int usage;

    public ShaderBuffer(int target, int usage) {
        ID = glGenBuffers();
        this.target = target;
        this.usage = usage;
    }

    public ShaderBuffer(int target, int usage, int ID) {
        this.ID = ID;
        this.target = target;
        this.usage = usage;
    }

    public void setUsage(int usage) {
        this.usage = usage;
    }
    
    public void setSize(int size) {
        glBindBuffer(target, ID);
        glBufferData(target, size, usage);
    }

    public void uploadData(int[] data) {
        glBindBuffer(target, ID);
        glBufferData(target, data, usage);
    }

    public void uploadData(long[] data) {
        glBindBuffer(target, ID);
        glBufferData(target, data, usage);
    }

    public void uploadData(float[] data) {
        glBindBuffer(target, ID);
        glBufferData(target, data, usage);
    }

    public void uploadData(ByteBuffer data) {
        glBindBuffer(target, ID);
        glBufferData(target, data, usage);
    }

    public void uploadPartialData(int[] data, int offset) {
        glBindBuffer(target, ID);
        glBufferSubData(target, (long) offset << 2l, data);
    }

    public void uploadPartialData(long[] data, int offset) {
        glBindBuffer(target, ID);
        glBufferSubData(target, (long) offset << 3l, data);
    }

    public void uploadPartialData(float[] data, int offset) {
        glBindBuffer(target, ID);
        glBufferSubData(target, (long) offset << 2l, data);
    }

    public void uploadPartialData(ByteBuffer data, long offset) {
        glBindBuffer(target, ID);
        glBufferSubData(target, offset, data);
    }
    
    public ByteBuffer map(int access) {
        glBindBuffer(target, ID);
        return glMapBuffer(target, access);
    }
    
    public ByteBuffer mapRange(long offset, long length, int access) {
        glBindBuffer(target, ID);
        return glMapBufferRange(target, offset, length, access);
    }
    
    public void unmap() {
        glBindBuffer(target, ID);
        glUnmapBuffer(target);
    }

    public void bind() {
        glBindBuffer(target, ID);
    }

    public void delete() {
        glDeleteBuffers(ID);
    }

}
