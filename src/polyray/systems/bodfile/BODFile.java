package polyray.systems.bodfile;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

public class BODFile {

    public static final ValueFactory<HashMap> OBJECT = new ValueFactory<>(HashMap.class, (ptr, size, f) -> {
        int count = size / 10;
        HashMap<String, ValueRef> obj = new HashMap<>(count);
        for (int i = 0; i < count; i++) {
            int index = ptr.getInt();
            ValueRef keyRef = new ValueRef(2, index);
            String key = f.getValue(keyRef);
            obj.put(key, new ValueRef(ptr.getShort() & 0xFFFF, ptr.getInt()));
        }
        return obj;
    }, (raw, f) -> {
        @SuppressWarnings("unchecked")
        HashMap<String, ValueRef> map = (HashMap<String, ValueRef>) raw;
        ObjectMap<String> out = new ObjectMap<>();
        for (Map.Entry<String, ValueRef> entry : map.entrySet()) {
            out.put(entry.getKey(), f.getAll(entry.getValue()));
        }
        return out;
    });
    public static final ValueFactory<ValueRef[]> ARRAY = new ValueFactory<>(ValueRef[].class, (ptr, size, f) -> {
        int count = size / 6;
        ValueRef[] arr = new ValueRef[count];
        for (int i = 0; i < count; i++) {
            arr[i] = new ValueRef(ptr.getShort() & 0xFFFF, ptr.getInt());
        }
        return arr;
    }, (raw, f) -> {
        Object[] arr = new Object[raw.length];
        for (int i = 0; i < raw.length; i++) {
            arr[i] = f.getAll(raw[i]);
        }
        return arr;
    });
    public static final ValueFactory<String> STRING = new ValueFactory<>(String.class, (ptr, size, f) -> new String(ptr.array(), StandardCharsets.UTF_8));
    public static final ValueFactory<Float> FLOAT = new ValueFactory<>(Float.class, (ptr, size, f) -> ptr.getFloat());
    public static final ValueFactory<Integer> INT = new ValueFactory<>(Integer.class, (ptr, size, f) -> ptr.getInt());
    public static final ValueFactory<UUID> UUID = new ValueFactory<>(UUID.class, (ptr, size, f) -> new UUID(ptr.getLong(), ptr.getLong()));

    private final Pool[] pools;
    private ValueFactory<?>[] factories;
    public final ValueRef root;

    public BODFile(InputStream stream) throws IOException {
        BufferedInputStream in = new BufferedInputStream(stream);
        ByteBuffer header = ByteBuffer.allocate(12).order(ByteOrder.LITTLE_ENDIAN);
        in.read(header.array());
        long magic = header.getLong();
        if (magic != 0xECA123B1CEFFAAFFl) {
        }
        pools = new Pool[header.getShort() & 0xFFFF];
        ByteBuffer poolDescs = ByteBuffer.allocate(pools.length * 16).order(ByteOrder.LITTLE_ENDIAN);
        in.read(poolDescs.array());
        for (int i = 0; i < pools.length; i++) {
            int stride = poolDescs.getShort() & 0xFFFF;
            poolDescs.getShort();
            int count = poolDescs.getInt();
            poolDescs.getInt();
            int length = poolDescs.getInt();
            pools[i] = new Pool(stride, count, length);
        }
        ByteBuffer rootBuffer = ByteBuffer.allocate(6).order(ByteOrder.LITTLE_ENDIAN);
        in.read(rootBuffer.array());
        root = new ValueRef(rootBuffer.getShort() & 0xFFFF, rootBuffer.getInt());
        for (Pool p : pools) {
            if (p.stride == 0) {
                ByteBuffer lutBuffer = ByteBuffer.allocate(4 * (p.count + 1)).order(ByteOrder.LITTLE_ENDIAN);
                in.read(lutBuffer.array());
                p.lut = new int[p.count + 1];
                for (int i = 0; i < p.count + 1; i++) {
                    p.lut[i] = lutBuffer.getInt();
                }
            }
            p.data = in.readNBytes(p.length);
        }
    }

    public BODFile(File file) throws IOException {
        this(new FileInputStream(file));
    }

    public void setValueFactories(ValueFactory<?>... factories) {
        this.factories = factories;
    }

    @SuppressWarnings("unchecked")
    public <T> T getValue(ValueRef ref) {
        Pool p = pools[ref.poolId];
        int offset;
        int size;
        if (p.lut == null) {
            offset = ref.index * p.stride;
            size = p.stride;
        } else {
            offset = p.lut[ref.index];
            size = p.lut[ref.index + 1] - offset;
        }
        byte[] buffer = new byte[size];
        System.arraycopy(p.data, offset, buffer, 0, size);
        return (T) factories[ref.poolId].get(ByteBuffer.wrap(buffer).order(ByteOrder.LITTLE_ENDIAN), size, this);
    }

    @SuppressWarnings("unchecked")
    public <T> T getAll(ValueRef ref) {
        Object raw = getValue(ref);
        if (factories[ref.poolId].resolver == null) {
            return (T) raw;
        }
        return (T) factories[ref.poolId].resolve(raw, this);
    }

    private static final class Pool {

        public final int stride;
        public final int count;
        public final int length;
        public int[] lut;
        public byte[] data;

        public Pool(int stride, int count, int length) {
            this.stride = stride;
            this.count = count;
            this.length = length;
        }
    }
}
