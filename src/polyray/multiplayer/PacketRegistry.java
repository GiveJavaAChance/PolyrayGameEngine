package polyray.multiplayer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.function.Supplier;

public class PacketRegistry {

    private static final ArrayList<Supplier<MultiplayerPacket>> constructors = new ArrayList<>();
    private static final HashMap<Class<? extends MultiplayerPacket>, Integer> classIds = new HashMap<>();

    public static synchronized int register(Class<? extends MultiplayerPacket> clazz, Supplier<MultiplayerPacket> constructor) {
        Integer d = classIds.get(clazz);
        if(d != null) {
            return d;
        }
        int id = constructors.size();
        constructors.add(constructor);
        classIds.put(clazz, id);
        return id;
    }

    public static MultiplayerPacket create(int id) {
        return constructors.get(id).get();
    }

    public static int getId(Class<? extends MultiplayerPacket> clazz) {
        Integer id = classIds.get(clazz);
        if (id == null) {
            throw new IllegalArgumentException("Class not registered: " + clazz.getName());
        }
        return id;
    }
}
