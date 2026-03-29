package polyray.systems.bodfile;

import java.nio.ByteBuffer;

public class ValueRef {

    public final int poolId;
    public final int index;

    public ValueRef(int poolId, int index) {
        this.poolId = poolId;
        this.index = index;
    }

    public void write(ByteBuffer buffer) {
        buffer.putShort((short) this.poolId);
        buffer.putInt(this.index);
    }
}
