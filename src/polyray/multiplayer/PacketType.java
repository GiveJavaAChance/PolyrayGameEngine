package polyray.multiplayer;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.util.function.Consumer;
import java.util.function.IntConsumer;

public class PacketType<T> {

    public final String tag;

    public final StatelessSerializer statelessSerializer;
    public final StatelessDeserializer statelessDeserializer;

    public final ObjectSerializer<T> objectSerializer;
    public final ObjectDeserializer<T> objectDeserializer;
    public final Consumer<T> objectHandler;

    public PacketType(String tag, StatelessSerializer statelessSerializer, StatelessDeserializer statelessDeserializer) {
        this.tag = tag;
        this.statelessSerializer = statelessSerializer;
        this.statelessDeserializer = statelessDeserializer;
        this.objectSerializer = null;
        this.objectDeserializer = null;
        this.objectHandler = null;
    }

    public PacketType(String tag, ObjectSerializer<T> objectSerializer, ObjectDeserializer<T> objectDeserializer, Consumer<T> objectHandler) {
        this.tag = tag;
        this.statelessSerializer = null;
        this.statelessDeserializer = null;
        this.objectSerializer = objectSerializer;
        this.objectDeserializer = objectDeserializer;
        this.objectHandler = objectHandler;
    }

    public Runnable deserialize(int clientID, ByteReader reader) {
        if (objectDeserializer == null) {
            try {
                int len = reader.getInt();
                byte[] data = reader.getBytes(len);
                ByteReader r = new ByteReader(new ByteArrayInputStream(data));
                return () -> {
                    try {
                        statelessDeserializer.deserialize(clientID, r);
                    } catch (IOException e) {
                    }
                };
            } catch (IOException e) {
            }
        }
        try {
            T obj = objectDeserializer.deserialize(clientID, reader);
            return () -> objectHandler.accept(obj);
        } catch (IOException e) {
        }
        return null;
    }
    
    public static PacketType message(String msg, IntConsumer onReceive) {
        return new PacketType(msg, out -> {}, (clientID, in) -> onReceive.accept(clientID));
    }
}
