package polyray.multiplayer;

import java.io.IOException;

@FunctionalInterface
public interface ObjectSerializer<T> {

    void serialize(T obj, ByteWriter out) throws IOException;
}
