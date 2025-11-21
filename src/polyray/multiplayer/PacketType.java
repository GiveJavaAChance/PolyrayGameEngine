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
    private final boolean isDirect;

    private PacketType(String tag, StatelessSerializer statelessSerializer, StatelessDeserializer statelessDeserializer, ObjectSerializer<T> objectSerializer, ObjectDeserializer<T> objectDeserializer, Consumer<T> objectHandler, boolean isDirect) {
        this.tag = tag;
        this.statelessSerializer = statelessSerializer;
        this.statelessDeserializer = statelessDeserializer;
        this.objectSerializer = objectSerializer;
        this.objectDeserializer = objectDeserializer;
        this.objectHandler = objectHandler;
        this.isDirect = isDirect;
    }

    public PacketType(String tag, StatelessSerializer statelessSerializer, StatelessDeserializer statelessDeserializer) {
        this(tag, statelessSerializer, statelessDeserializer, null, null, null, false);
    }

    public PacketType(String tag, ObjectSerializer<T> objectSerializer, ObjectDeserializer<T> objectDeserializer, Consumer<T> objectHandler) {
        this(tag, null, null, objectSerializer, objectDeserializer, objectHandler, false);
    }

    public Runnable deserialize(int clientID, ByteReader reader) {
        if (objectDeserializer == null) {
            try {
                int len = reader.getInt();
                if (isDirect) {
                    statelessDeserializer.deserialize(clientID, reader);
                    return null;
                }
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
            if (isDirect) {
                objectHandler.accept(obj);
                return null;
            }
            return () -> objectHandler.accept(obj);
        } catch (IOException e) {
        }
        return null;
    }

    public static PacketType message(String msg, IntConsumer onReceive) {
        return new PacketType(msg, out -> {}, (clientID, in) -> onReceive.accept(clientID));
    }

    public static PacketType direct(String tag, StatelessSerializer statelessSerializer, StatelessDeserializer statelessDeserializer) {
        return new PacketType<>(tag, statelessSerializer, statelessDeserializer, null, null, null, true);
    }

    public static <T> PacketType direct(String tag, ObjectSerializer<T> objectSerializer, ObjectDeserializer<T> objectDeserializer, Consumer<T> objectHandler) {
        return new PacketType<>(tag, null, null, objectSerializer, objectDeserializer, objectHandler, true);
    }
}
