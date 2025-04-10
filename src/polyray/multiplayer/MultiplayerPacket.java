package polyray.multiplayer;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;

public abstract class MultiplayerPacket {

    private int ID = -1;

    void setId(int id) {
        this.ID = id;
    }

    public int getId() {
        return ID;
    }

    public abstract void read(BufferedInputStream in) throws IOException;

    public abstract void write(BufferedOutputStream out) throws IOException;
}
