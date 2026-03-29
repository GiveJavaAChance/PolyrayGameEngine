package polyray.systems.bodfile;

import java.nio.ByteBuffer;

@FunctionalInterface
public interface ValueReader<T> {

    public T get(ByteBuffer ptr, int size, BODFile file);
}
