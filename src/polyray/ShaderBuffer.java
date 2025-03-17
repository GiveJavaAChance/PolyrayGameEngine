package polyray;

import java.nio.ByteBuffer;
import static org.lwjgl.opengl.GL15.*;

public class ShaderBuffer {

    public final int ID;
    public final int target;
    private int usage;

    public ShaderBuffer(int target, int usage) {
        ID = glGenBuffers();
        this.target = target;
        this.usage = usage;
    }

    public void setUsage(int usage) {
        this.usage = usage;
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

    public void delete() {
        glDeleteBuffers(ID);
    }

}
