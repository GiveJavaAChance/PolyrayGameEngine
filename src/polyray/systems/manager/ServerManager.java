package polyray.systems.manager;

import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import polyray.multiplayer.Multiplayer;
import polyray.multiplayer.MultiplayerPacket;

public class ServerManager {

    private static MultiplayerServer server;
    private static final HashMap<Class<? extends MultiplayerPacket>, ArrayList<PacketHandler<?>>> handlers = new HashMap<>();

    public static final boolean initialize(String serverHost, int port) {
        try {
            server = new MultiplayerServer(serverHost, port);
            server.start();
            return true;
        } catch (IOException e) {
            server = null;
            return false;
        }
    }

    public static final <T extends MultiplayerPacket> void registerHandler(Class<T> type, PacketHandler<T> handler) {
        handlers.computeIfAbsent(type, k -> new ArrayList<>()).add(handler);
    }

    public static final void send(MultiplayerPacket p) {
        if (server != null) {
            server.sendPacket(p);
        }
    }

    public static final void update() {
        if (server != null) {
            server.pollPackets();
        }
    }

    public static final void exit() {
        if (server != null) {
            try {
                server.disconnect();
            } catch (IOException e) {
            }
        }
    }

    public static final int getClientID() {
        if (server == null) {
            return 0;
        }
        return server.getClientID();
    }
    
    public static final boolean isOffline() {
        return server == null;
    }

    public static interface PacketHandler<T extends MultiplayerPacket> {

        public void handle(T packet, int ID);
    }

    private static class MultiplayerServer extends Multiplayer {

        public MultiplayerServer(String serverHost, int serverPort) throws IOException {
            super(serverHost, serverPort);
        }

        public synchronized void sendPacket(MultiplayerPacket p) {
            try {
                send(p);
            } catch (IOException e) {
                System.err.println("Error sending packet: " + e);
            }
        }

        @Override
        public void onReceive(MultiplayerPacket p, int ID) {
            ArrayList<PacketHandler<?>> h = handlers.get(p.getClass());
            if (h == null) {
                return;
            }
            for (PacketHandler<?> handler : h) {
                handlePacket(handler, p, ID);
            }
        }

        @SuppressWarnings("unchecked")
        private <T extends MultiplayerPacket> void handlePacket(PacketHandler<T> handler, MultiplayerPacket p, int ID) {
            handler.handle((T) p, ID);
        }
    }
}
