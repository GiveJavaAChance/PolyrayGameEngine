package polyray.multiplayer;

import java.io.IOException;

@FunctionalInterface
public interface StatelessSerializer {

    void serialize(ByteWriter out) throws IOException;
}
