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
    private final ArrayList<MultiplayerPacket> packets;
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

    public abstract void onReceive(MultiplayerPacket p);

    public void pollPackets() {
        ArrayList<MultiplayerPacket> packFrame;
        synchronized (packetLock) {
            packFrame = new ArrayList<>(packets);
            packets.clear();
        }
        for (MultiplayerPacket p : packFrame) {
            onReceive(p);
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
                    p.setId(clientID);
                    p.read(in);
                    synchronized (packetLock) {
                        packets.add(p);
                    }
                }
            } catch (IOException e) {
            }
        }).start();
    }
}
