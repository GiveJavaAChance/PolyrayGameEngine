package polyray.multiplayer;

import java.io.IOException;

@FunctionalInterface
public interface ObjectDeserializer<T> {

    T deserialize(int clientID, ByteReader in) throws IOException;
}
