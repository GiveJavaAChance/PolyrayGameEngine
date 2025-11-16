package polyray.multiplayer;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

public class ByteWriter {

    private final OutputStream out;
    private ByteOrder order;

    public ByteWriter(OutputStream out) {
        this.out = out;
        this.order = ByteOrder.BIG_ENDIAN;
    }

    public void setOrder(ByteOrder order) {
        this.order = order;
    }

    public void writeByte(byte b) throws IOException {
        out.write(new byte[]{b});
    }

    public void writeBytes(byte... bytes) throws IOException {
        out.write(bytes);
    }

    public void writeShort(short s) throws IOException {
        out.write(alloc(2).putShort(s).array());
    }

    public void writeShorts(short... shorts) throws IOException {
        ByteBuffer buffer = alloc(shorts.length << 1);
        buffer.asShortBuffer().put(shorts);
        out.write(buffer.array());
    }

    public void writeInt(int i) throws IOException {
        out.write(alloc(4).putInt(i).array());
    }

    public void writeInts(int... ints) throws IOException {
        ByteBuffer buffer = alloc(ints.length << 2);
        buffer.asIntBuffer().put(ints);
        out.write(buffer.array());
    }

    public void writeLong(long l) throws IOException {
        out.write(alloc(8).putLong(l).array());
    }

    public void writeLongs(long... longs) throws IOException {
        ByteBuffer buffer = alloc(longs.length << 3);
        buffer.asLongBuffer().put(longs);
        out.write(buffer.array());
    }

    public void writeFloat(float f) throws IOException {
        out.write(alloc(4).putFloat(f).array());
    }

    public void writeFloats(float... floats) throws IOException {
        ByteBuffer buffer = alloc(floats.length << 2);
        buffer.asFloatBuffer().put(floats);
        out.write(buffer.array());
    }

    public void writeDouble(double d) throws IOException {
        out.write(alloc(8).putDouble(d).array());
    }

    public void writeDoubles(double... doubles) throws IOException {
        ByteBuffer buffer = alloc(doubles.length << 3);
        buffer.asDoubleBuffer().put(doubles);
        out.write(buffer.array());
    }

    public void writeString(String str) throws IOException {
        byte[] bytes = str.getBytes(StandardCharsets.UTF_8);
        writeInt(bytes.length);
        writeBytes(bytes);
    }

    private ByteBuffer alloc(int count) {
        return ByteBuffer.allocate(count).order(order);
    }
}
