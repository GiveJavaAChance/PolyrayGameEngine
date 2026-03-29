package polyray.systems.bodfile.json;

import java.io.BufferedOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Map.Entry;
import java.util.UUID;
import java.util.function.Consumer;
import java.util.function.Function;
import polyray.systems.bodfile.ValueRef;
import polyray.systems.parsing.CST;
import polyray.systems.parsing.TokenRegistry;
import polyray.systems.parsing.Tokenizer;

public class JSONCompiler {

    public static final ValueCodec<HashMap> OBJECT_TYPE = new ValueCodec<>(0, HashMap.class, "{", "}") {
        @Override
        public HashMap parse(CST cst, Function<CST, Object> parser) {
            HashMap<Object, Object> obj = new HashMap<>();
            int idx = 0;
            while (idx < cst.children.size()) {
                CST key = cst.children.get(idx++);
                idx++;
                CST value = cst.children.get(idx++);
                obj.put(parser.apply(key), parser.apply(value));
                if (idx < cst.children.size()) {
                    idx++;
                }
            }
            return obj;
        }

        @Override
        public void collectChildValues(HashMap value, Consumer<Object> collector) {
            @SuppressWarnings("unchecked")
            HashMap<Object, Object> map = (HashMap<Object, Object>) value;
            for (Entry<Object, Object> e : map.entrySet()) {
                collector.accept(e.getKey());
                collector.accept(e.getValue());
            }
        }

        @Override
        public int byteSize(HashMap value) {
            return value.size() * 10;
        }

        @Override
        public void write(ByteBuffer buffer, HashMap value, Function<Object, ValueRef> createReference) {
            @SuppressWarnings("unchecked")
            HashMap<Object, Object> map = (HashMap<Object, Object>) value;
            for (Entry<Object, Object> e : map.entrySet()) {
                ValueRef key = createReference.apply(e.getKey());
                buffer.putInt(key.index);
                createReference.apply(e.getValue()).write(buffer);
            }
        }
    };
    public static final ValueCodec<ArrayList> ARRAY_TYPE = new ValueCodec<>(0, ArrayList.class, "[", "]") {
        @Override
        public ArrayList parse(CST cst, Function<CST, Object> parser) {
            ArrayList<Object> arr = new ArrayList<>();
            int idx = 0;
            while (idx < cst.children.size()) {
                CST value = cst.children.get(idx++);
                arr.add(parser.apply(value));
                if (idx < cst.children.size()) {
                    idx++;
                }
            }
            return arr;
        }

        @Override
        public void collectChildValues(ArrayList value, Consumer<Object> collector) {
            @SuppressWarnings("unchecked")
            ArrayList<Object> arr = (ArrayList<Object>) value;
            for (Object v : arr) {
                collector.accept(v);
            }
        }

        @Override
        public int byteSize(ArrayList value) {
            return value.size() * 6;
        }

        @Override
        public void write(ByteBuffer buffer, ArrayList value, Function<Object, ValueRef> createReference) {
            @SuppressWarnings("unchecked")
            ArrayList<Object> arr = (ArrayList<Object>) value;
            for (Object v : arr) {
                createReference.apply(v).write(buffer);
            }
        }
    };
    public static final ValueCodec<String> STRING_TYPE = new ValueCodec<>(0, String.class, TokenRegistry.STRING) {
        @Override
        public String parse(CST cst, Function<CST, Object> parser) {
            return descape(cst.token.value);
        }

        @Override
        public int byteSize(String value) {
            return value.getBytes(StandardCharsets.UTF_8).length;
        }

        @Override
        public void write(ByteBuffer buffer, String value, Function<Object, ValueRef> createReference) {
            buffer.put(value.getBytes(StandardCharsets.UTF_8));
        }
    };
    public static final ValueCodec<Integer> INT_TYPE = new ValueCodec<>(4, Integer.class, TokenRegistry.INT) {
        @Override
        public Integer parse(CST cst, Function<CST, Object> parser) {
            return Integer.valueOf(cst.token.value.toLowerCase());
        }

        @Override
        public int byteSize(Integer value) {
            return 4;
        }

        @Override
        public void write(ByteBuffer buffer, Integer value, Function<Object, ValueRef> createReference) {
            buffer.putInt(value);
        }
    };
    public static final ValueCodec<Float> FLOAT_TYPE = new ValueCodec<>(4, Float.class, TokenRegistry.FLOAT) {
        @Override
        public Float parse(CST cst, Function<CST, Object> parser) {
            return Float.valueOf(cst.token.value);
        }

        @Override
        public int byteSize(Float value) {
            return 4;
        }

        @Override
        public void write(ByteBuffer buffer, Float value, Function<Object, ValueRef> createReference) {
            buffer.putFloat(value);
        }
    };
    public static final ValueCodec<UUID> UUID_TYPE = new ValueCodec<>(16, UUID.class, TokenRegistry.UUID) {
        @Override
        public UUID parse(CST cst, Function<CST, Object> parser) {
            return UUID.fromString(cst.token.value);
        }

        @Override
        public int byteSize(UUID value) {
            return 16;
        }

        @Override
        public void write(ByteBuffer buffer, UUID value, Function<Object, ValueRef> createReference) {
            buffer.putLong(value.getLeastSignificantBits());
            buffer.putLong(value.getMostSignificantBits());
        }
    };
    public static final ValueCodec<?>[] DEFAULT_JSON = {OBJECT_TYPE, ARRAY_TYPE, STRING_TYPE, FLOAT_TYPE, INT_TYPE};

    private final HashMap<Class<?>, Integer> idMap = new HashMap<>();
    private final ArrayList<ValueCodec<?>> codecs = new ArrayList<>();
    private final HashMap<Integer, Integer> tokenIdMap = new HashMap<>();
    private final ArrayList<Integer> ocList = new ArrayList<>();
    private final ArrayList<LinkedHashSet<Object>> pools = new ArrayList<>();

    private final TokenRegistry reg;

    public JSONCompiler() {
        this.reg = new TokenRegistry();
        reg.registerToken(",");
        reg.registerToken(":");
    }

    public void register(ValueCodec<?>... codecs) {
        for (ValueCodec<?> codec : codecs) {
            boolean isOc = (codec.openToken != null && codec.closeToken != null) || (codec.openParser != null && codec.closeParser != null);
            int openId = codec.openToken != null ? reg.registerToken(codec.openToken) : reg.registerMatch(codec.openParser);
            int id = pools.size();
            if (isOc) {
                int closeId = codec.closeToken != null ? reg.registerToken(codec.closeToken) : reg.registerMatch(codec.closeParser);
                ocList.add(openId);
                ocList.add(closeId);
            }
            this.codecs.add(codec);
            idMap.put(codec.clazz, id);
            tokenIdMap.put(openId, id);
            pools.add(new LinkedHashSet<>());
        }
    }

    public void compileString(String json, File output) throws IOException {
        int[] oc = new int[ocList.size()];
        for (int i = 0; i < ocList.size(); i++) {
            oc[i] = ocList.get(i);
        }
        CST c = CST.buildTree(new Tokenizer(reg, json).tokenize(), oc);
        Object v = parse(c.children.get(0));
        collect(v);
        writeFile(output, v);
    }

    public void compileFile(File json, File output) throws IOException {
        compileString(new String(Files.readAllBytes(json.toPath())), output);
    }

    private Object parse(CST cst) {
        int type = cst.token.type;
        ValueCodec<?> codec = codecs.get(tokenIdMap.get(type));
        if (codec == null) {
            return cst.token.value;
        }
        return codec.parse(cst, this::parse);
    }

    private void collect(Object v) {
        int poolId = idMap.get(v.getClass());
        pools.get(poolId).add(v);
        codecs.get(poolId)._collectChildValues(v, this::collect);
    }

    private void writeFile(File file, Object root) throws IOException {
        BufferedOutputStream out = new BufferedOutputStream(new FileOutputStream(file));
        ByteBuffer header = ByteBuffer.allocate(12 + 16 * pools.size() + 6).order(ByteOrder.LITTLE_ENDIAN);;
        header.putLong(0xECA123B1CEFFAAFFl);
        header.putShort((short) pools.size());
        header.putShort((short) 0);
        int[] poolSize = new int[pools.size()];
        int off_data = header.array().length;
        int i = 0;
        for (LinkedHashSet<Object> pool : pools) {
            ValueCodec<?> codec = codecs.get(i);
            int size;
            int poolLength;
            if (codec.stride != 0) {
                size = codec.stride * pool.size();
                poolLength = size;
            } else {
                size = 4 * (pool.size() + 1);
                poolLength = size;
                for (Object v : pool) {
                    size += codec._byteSize(v);
                }
                poolLength = size - poolLength;
            }
            poolSize[i++] = size;
            header.putShort((short) codec.stride);
            header.putShort((short) 0);

            header.putInt(pool.size());
            header.putInt(off_data);
            header.putInt(poolLength);
            off_data += size;
        }
        createReference(root).write(header);
        out.write(header.array());
        i = 0;
        for (LinkedHashSet<Object> pool : pools) {
            ValueCodec<?> codec = codecs.get(i);
            ByteBuffer buffer = ByteBuffer.allocate(poolSize[i]).order(ByteOrder.LITTLE_ENDIAN);
            int offset = 0;
            if (codec.stride == 0) {
                for (Object v : pool) {
                    buffer.putInt(offset);
                    offset += codec._byteSize(v);
                }
                buffer.putInt(offset);
            }
            for (Object v : pool) {
                codec._write(buffer, v, this::createReference);
            }
            out.write(buffer.array());
            i++;
        }
        out.close();
    }

    private static int indexOf(LinkedHashSet<Object> set, Object e) {
        int i = 0;
        for (Object t : set) {
            if (t.equals(e)) {
                return i;
            }
            i++;
        }
        return -1;
    }

    private ValueRef createReference(Object v) {
        int poolId = idMap.get(v.getClass());
        return new ValueRef(poolId, indexOf(pools.get(poolId), v));
    }

    private static String descape(String str) {
        StringBuilder b = new StringBuilder(str.length() - 2);
        int count = 0;
        for (char c : str.substring(1, str.length() - 1).toCharArray()) {
            if (c == '\\') {
                if ((count & 1) != 0) {
                    b.append(c);
                }
                count++;
            } else {
                b.append(c);
                count = 0;
            }
        }
        return b.toString();
    }
}
