import java.io.BufferedOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import polyray.multiplayer.Multiplayer;
import polyray.multiplayer.MultiplayerPacket;
import polyray.multiplayer.PacketRegistry;
import polyray.multiplayer.ServerStream;

public class MultiplayerClient {

    public Multiplayer m;

    public MultiplayerClient() throws IOException, InterruptedException {
        this.m = new Multiplayer("localhost", 3050) {
            @Override
            public void onReceive(MultiplayerPacket p, int ID) {
                System.out.println(ID);
                if (p instanceof RectanglePacket pd) {
                    System.out.println(pd.width + " " + pd.height);
                }
            }
        };

        PacketRegistry.register(RectanglePacket.class, RectanglePacket::new);
        m.start();

        RectanglePacket p = new RectanglePacket();
        p.width = (int) (Math.random() * 100.0f) + 100;
        p.height = (int) (Math.random() * 100.0f) + 100;
        System.out.println("This size: " + p.width + ", " + p.height);

        while (true) {
            m.send(p);
            Thread.sleep(1000);
            m.pollPackets();
            Thread.sleep(1000);
        }
    }

    public static void main(String[] args) throws IOException, InterruptedException {
        MultiplayerClient p = new MultiplayerClient();
    }

    public static class RectanglePacket implements MultiplayerPacket {

        public int width, height;

        @Override
        public void read(ServerStream in) throws IOException {
            ByteBuffer buffer = in.read(8);
            this.width = buffer.getInt();
            this.height = buffer.getInt();
        }

        @Override
        public void write(BufferedOutputStream out) throws IOException {
            out.write(ByteBuffer.allocate(8).putInt(width).putInt(height).array());
        }

    }
}
