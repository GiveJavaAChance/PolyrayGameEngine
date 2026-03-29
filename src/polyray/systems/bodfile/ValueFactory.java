package polyray.systems.bodfile;

import java.nio.ByteBuffer;
import java.util.function.BiFunction;

public class ValueFactory<T> {

    public final Class<T> type;
    public final ValueReader<T> func;
    public final BiFunction<T, BODFile, Object> resolver;

    public ValueFactory(Class<T> type, ValueReader<T> func) {
        this.type = type;
        this.func = func;
        this.resolver = null;
    }

    public ValueFactory(Class<T> type, ValueReader<T> func, BiFunction<T, BODFile, Object> resolver) {
        this.type = type;
        this.func = func;
        this.resolver = resolver;
    }

    @SuppressWarnings("unchecked")
    public Object resolve(Object obj, BODFile file) {
        return resolver.apply((T) obj, file);
    }

    public T get(ByteBuffer ptr, int size, BODFile file) {
        return func.get(ptr, size, file);
    }
}
