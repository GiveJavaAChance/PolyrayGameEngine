import java.io.IOException;
import java.util.function.Consumer;
import polyray.multiplayer.Multiplayer;
import polyray.multiplayer.ObjectDeserializer;
import polyray.multiplayer.ObjectSerializer;
import polyray.multiplayer.PacketRegistry;
import polyray.multiplayer.PacketType;

public class MultiplayerClient {

    public MultiplayerClient() {
        // This is a showcase of the new (hopefully much better) multiplayer networking system
        
        // The previous design allowed the developer to register packet classes
        // whenever, but with this new design, you're forced to define/register
        // all packet types at the same time to avoid packet id missalignment
        // (so one client interprets a packet ID as being something completely
        // different from another client).
        //
        // Another thing that was quite annoying with the old design was the fact
        // that you had to create new MultiplayerPacket subclasses every time you
        // needed a new packet. Even though that new packet data was identical
        // to some other packet. You couldn't reuse the same packet class for
        // something different since you tied a the packet class to a id. So for
        // this design, I changed it to a tag-based system to greately improve
        // reusability, since packet ids are now tied to a tag string, instead
        // of the packet type.
        
        // To begin, you need to create a PackretRegistry where you define all
        // packet types and tags. The PacketRegistry is immutable, so you need
        // to specify ALL packets immediately.
        
        // There are two types of packets that can be made, packets that directly
        // modify state somewhere (stateless packets), and packets that creates objects (object packets). For this
        // showcase, I'll show the object creation packets for simplicity,
        // although I'd recomend for any larger game, to only use stateless packets
        // as you avoid tons of unnecessary object allocations.
        
        // For the object packet, you need to specify how to serialize and
        // deserialize the object, as well as a handler that receives the object
        // once it's been deserialized and polled
        ObjectSerializer<Rect> ser = (rect, out) -> out.writeInts(rect.width, rect.height);
        ObjectDeserializer<Rect> des = (clientID, in) -> new Rect(in.getInt(), in.getInt());
        Consumer<Rect> handler = rect -> {
            System.out.println("Received rectangle with dimensions: " + rect.width + "x" + rect.height + "!");
        };
        
        // Then it's just a matter of putting them all together into one packet
        // type and assigning a tag to it:
        PacketType<Rect> type = new PacketType<>("rect", ser, des, handler);
        
        // And you now have a packet type ready!
        // Just create a PacketRegistry with the types you've defined (in this
        // case, the "rect" type):
        PacketRegistry reg = new PacketRegistry(type);
        
        // A little note is that I've designed the registration to be as compact
        // as possible. So you can even define that "rect" type in one line!
        //  PacketRegistry reg = new PacketRegistry(new PacketType<>("rect", (rect, out) -> out.writeInts(rect.width, rect.height), (clientID, in) -> new Rect(in.getInt(), in.getInt()), rect -> {
        //      System.out.println("Received rectangle with dimensions: " + rect.width + "x" + rect.height + "!");
        //  }));
        
        
        // Now, once you got the PacketRegistry done, simply create a Multiplayer
        // instance and start it to begin sending and receiving packets
        Multiplayer m;
        try {
            m = new Multiplayer(reg, "localhost", 3050);
        } catch (IOException e) {
            System.err.println("Failed to create server!");
            return;
        }
        m.start();

        // An example Rect object to send
        Rect r = new Rect((int) (Math.random() * 100.0f) + 100, (int) (Math.random() * 100.0f) + 100);
        System.err.println("Your ractangle size: " + r.width + ", " + r.height);

        while (true) {
            // Here, you specify the packet type using the tag (in this case
            // "rect") and pass a object to serialize. This is why the new system
            // is much better, since you could now define another packet type
            // that uses the same Rect object for data, but it has a different
            // tag and handler. No need to define a new Rect class!
            m.send("rect", r);
            
            try {
                // Just sleeping to make it easier to read the terminal
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
            
            // Here, you poll queued packets so handlers run on the main thread.
            // For object packets, deserialization happens immediately on
            // the network thread, but the handler is executed only when
            // packets are polled
            m.pollPackets();
            
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
        }
    }

    public static void main(String[] args) {
        MultiplayerClient p = new MultiplayerClient();
    }

    public static class Rect {

        public int width, height;

        public Rect(int width, int height) {
            this.width = width;
            this.height = height;
        }
    }
}
