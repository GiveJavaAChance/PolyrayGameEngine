package polyray.multiplayer;

import java.io.BufferedInputStream;
import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class Multiplayer {

    private final PacketRegistry registry;
    private final int ID;
    private final Socket socket;
    private final BufferedInputStream in;
    private final BufferedOutputStream out;
    private boolean listening;
    private final ArrayList<Runnable> packets;
    private final Object packetLock = new Object();

    public Multiplayer(PacketRegistry registry, String serverHost, int serverPort) throws IOException {
        this.registry = registry;
        this.socket = new Socket(serverHost, serverPort);
        this.in = new BufferedInputStream(socket.getInputStream());
        this.out = new BufferedOutputStream(socket.getOutputStream());
        this.listening = false;
        this.packets = new ArrayList<>();
        byte[] buffer = new byte[4];
        ByteReader.readFully(in, buffer);
        this.ID = ByteBuffer.wrap(buffer).getInt();
    }

    public void start() {
        listen();
    }

    public void send(String tag) {
        try {
            int packetID = registry.getID(tag);
            ByteArrayOutputStream baos = new ByteArrayOutputStream();
            PacketType type = registry.getType(packetID);
            type.statelessSerializer.serialize(new ByteWriter(baos));
            byte[] data = baos.toByteArray();
            ByteBuffer header = ByteBuffer.allocate(12);
            header.putInt(ID);
            header.putInt(packetID);
            header.putInt(data.length);
            out.write(header.array());
            out.write(data);
            out.flush();
        } catch (Exception e) {
            throw new RuntimeException("Failed to send packet \"" + tag + "\", reason: " + e);
        }
    }

    @SuppressWarnings("unchecked")
    public <T> void send(String tag, T obj) {
        try {
            int packetID = registry.getID(tag);
            ByteBuffer header = ByteBuffer.allocate(8);
            header.putInt(ID);
            header.putInt(packetID);
            out.write(header.array());
            PacketType type = registry.getType(packetID);
            ((ObjectSerializer<T>) type.objectSerializer).serialize(obj, new ByteWriter(out));
            out.flush();
        } catch (Exception e) {
            throw new RuntimeException("Failed to send packet \"" + tag + "\", reason: " + e);
        }
    }

    public void pollPackets() {
        ArrayList<Runnable> packFrame;
        synchronized (packetLock) {
            packFrame = new ArrayList<>(packets);
            packets.clear();
        }
        for (Runnable r : packFrame) {
            r.run();
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
            byte[] bytes = new byte[8];
            ByteBuffer header = ByteBuffer.wrap(bytes);
            while (!socket.isClosed()) {
                try {
                    ByteReader.readFully(in, bytes);
                } catch (IOException e) {
                    break;
                }
                int clientID = header.getInt(0);
                int packetID = header.getInt(4);
                Runnable r = registry.getType(packetID).deserialize(clientID, new ByteReader(in));
                synchronized (packetLock) {
                    packets.add(r);
                }
            }
        }).start();
    }

    public int getClientID() {
        return this.ID;
    }
}
