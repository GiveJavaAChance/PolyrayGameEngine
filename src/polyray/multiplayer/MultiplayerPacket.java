package polyray.multiplayer;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;

public interface MultiplayerPacket {

    public void read(BufferedInputStream in) throws IOException;

    public void write(BufferedOutputStream out) throws IOException;
}
