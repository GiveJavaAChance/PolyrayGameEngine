package polyray.multiplayer;

import java.io.IOException;

@FunctionalInterface
public interface StatelessDeserializer {

    void deserialize(int clientId, ByteReader in) throws IOException;
}
