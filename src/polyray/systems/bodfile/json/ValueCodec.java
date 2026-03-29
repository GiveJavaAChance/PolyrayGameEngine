package polyray.systems.bodfile.json;

import java.nio.ByteBuffer;
import java.util.function.Consumer;
import java.util.function.Function;
import polyray.systems.bodfile.ValueRef;
import polyray.systems.parsing.CST;
import polyray.systems.parsing.TokenParser;

public abstract class ValueCodec<T> {

    public final int stride;

    public final Class<T> clazz;
    public final String openToken;
    public final String closeToken;
    public final TokenParser openParser;
    public final TokenParser closeParser;

    private ValueCodec(int stride, Class<T> clazz, String openToken, String closeToken, TokenParser openParser, TokenParser closeParser) {
        this.stride = stride;
        this.clazz = clazz;
        this.openToken = openToken;
        this.closeToken = closeToken;
        this.openParser = openParser;
        this.closeParser = closeParser;
    }

    public ValueCodec(int stride, Class<T> clazz, String token) {
        this(stride, clazz, token, null, null, null);
    }

    public ValueCodec(int stride, Class<T> clazz, String openToken, String closeToken) {
        this(stride, clazz, openToken, closeToken, null, null);
    }

    public ValueCodec(int stride, Class<T> clazz, TokenParser parser) {
        this(stride, clazz, null, null, parser, null);
    }

    public ValueCodec(int stride, Class<T> clazz, TokenParser openParser, TokenParser closeParser) {
        this(stride, clazz, null, null, openParser, closeParser);
    }

    public abstract T parse(CST cst, Function<CST, Object> parser);

    public abstract int byteSize(T value);

    public abstract void write(ByteBuffer buffer, T value, Function<Object, ValueRef> createReference);

    public void collectChildValues(T value, Consumer<Object> collector) {
    }

    @SuppressWarnings("unchecked")
    public final int _byteSize(Object value) {
        return byteSize((T) value);
    }

    @SuppressWarnings("unchecked")
    public final void _write(ByteBuffer buffer, Object value, Function<Object, ValueRef> createReference) {
        write(buffer, (T) value, createReference);
    }

    @SuppressWarnings("unchecked")
    public final void _collectChildValues(Object value, Consumer<Object> collector) {
        collectChildValues((T) value, collector);
    }
}
