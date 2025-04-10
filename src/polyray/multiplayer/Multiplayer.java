package polyray.multiplayer;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public abstract class Multiplayer {

    private final int ID;
    private final Socket socket;
    private final BufferedInputStream in;
    private final BufferedOutputStream out;
    private boolean listening;
    private final ArrayList<Pack> packets;
    private final Object packetLock = new Object();

    public Multiplayer(String serverHost, int serverPort) throws IOException {
        this.socket = new Socket(serverHost, serverPort);
        this.in = new BufferedInputStream(socket.getInputStream());
        this.out = new BufferedOutputStream(socket.getOutputStream());
        this.listening = false;
        this.packets = new ArrayList<>();

        this.ID = ByteBuffer.wrap(in.readNBytes(4)).getInt();
    }

    public void start() {
        listen();
    }

    public void send(MultiplayerPacket p) throws IOException {
        ByteBuffer header = ByteBuffer.allocate(8);
        header.putInt(ID);
        header.putInt(PacketRegistry.getId(p.getClass()));
        out.write(header.array());
        p.write(out);
        out.flush();
    }

    public abstract void onReceive(MultiplayerPacket p, int ID);

    public void pollPackets() {
        ArrayList<Pack> packFrame;
        synchronized (packetLock) {
            packFrame = new ArrayList<>(packets);
            packets.clear();
        }
        for (Pack pack : packFrame) {
            onReceive(pack.p, pack.ID);
        }
    }

    public void disconnect() throws IOException {
        socket.close();
    }

    private void listen() {
        if (listening) {
            return;
        }
        listening = true;
        new Thread(() -> {
            try {
                while (!socket.isClosed()) {
                    ByteBuffer header = ByteBuffer.wrap(in.readNBytes(8));
                    int clientID = header.getInt();
                    int packetID = header.getInt();
                    MultiplayerPacket p = PacketRegistry.create(packetID);
                    p.read(in);
                    synchronized (packetLock) {
                        packets.add(new Pack(p, clientID));
                    }
                }
            } catch (IOException e) {
            }
        }).start();
    }
    
    public int getClientID() {
        return this.ID;
    }

    private static class Pack {

        public final MultiplayerPacket p;
        public final int ID;

        public Pack(MultiplayerPacket p, int ID) {
            this.p = p;
            this.ID = ID;
        }
    }
}
