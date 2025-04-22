package polyray.multiplayer;

import java.io.BufferedOutputStream;
import java.io.IOException;

public interface MultiplayerPacket {

    public void read(ServerStream in) throws IOException;

    public void write(BufferedOutputStream out) throws IOException;
}
