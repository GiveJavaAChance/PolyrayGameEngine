package polyray.multiplayer;

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;

public class ByteReader {

    private final InputStream in;
    private ByteOrder order;

    public ByteReader(InputStream in) {
        this.in = in;
        this.order = ByteOrder.BIG_ENDIAN;
    }

    public void setOrder(ByteOrder order) {
        this.order = order;
    }

    public byte getByte() throws IOException {
        return getBytes(1)[0];
    }

    public void getBytes(byte[] bytes) throws IOException {
        readFully(in, bytes);
    }

    public byte[] getBytes(int count) throws IOException {
        byte[] bytes = new byte[count];
        readFully(in, bytes);
        return bytes;
    }

    public ByteBuffer getByteBuffer(int count) throws IOException {
        return ByteBuffer.wrap(getBytes(count)).order(order);
    }

    public short getShort() throws IOException {
        return getByteBuffer(2).getShort();
    }

    public void getShorts(short[] shorts) throws IOException {
        getByteBuffer(shorts.length << 1).asShortBuffer().get(shorts);
    }

    public short[] getShorts(int count) throws IOException {
        short[] shorts = new short[count];
        getShorts(shorts);
        return shorts;
    }

    public int getInt() throws IOException {
        return getByteBuffer(4).getInt();
    }

    public void getInts(int[] ints) throws IOException {
        getByteBuffer(ints.length << 2).asIntBuffer().get(ints);
    }

    public int[] getInts(int count) throws IOException {
        int[] ints = new int[count];
        getInts(ints);
        return ints;
    }

    public long getLong() throws IOException {
        return getByteBuffer(8).getLong();
    }

    public void getLongs(long[] longs) throws IOException {
        getByteBuffer(longs.length << 3).asLongBuffer().get(longs);
    }

    public long[] getLongs(int count) throws IOException {
        long[] longs = new long[count];
        getLongs(longs);
        return longs;
    }

    public float getFloat() throws IOException {
        return getByteBuffer(4).getFloat();
    }

    public void getFloats(float[] floats) throws IOException {
        getByteBuffer(floats.length << 2).asFloatBuffer().get(floats);
    }

    public float[] getFloats(int count) throws IOException {
        float[] floats = new float[count];
        getFloats(floats);
        return floats;
    }

    public double getDouble() throws IOException {
        return getByteBuffer(8).getDouble();
    }

    public void getDoubles(double[] doubles) throws IOException {
        getByteBuffer(doubles.length << 3).asDoubleBuffer().get(doubles);
    }

    public double[] getDoubles(int count) throws IOException {
        double[] doubles = new double[count];
        getDoubles(doubles);
        return doubles;
    }

    public String getString() throws IOException {
        int len = getInt();
        return new String(getBytes(len), StandardCharsets.UTF_8);
    }

    public static void readFully(InputStream in, byte[] buffer) throws IOException {
        int n = 0;
        while (n < buffer.length) {
            int r = in.read(buffer, n, buffer.length - n);
            if (r < 0) {
                throw new EOFException();
            }
            n += r;
        }
    }
}
