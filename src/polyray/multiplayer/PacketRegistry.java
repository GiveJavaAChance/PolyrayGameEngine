package polyray.multiplayer;

import java.util.HashMap;

public class PacketRegistry {

    private final HashMap<Integer, PacketType> reg;
    private final HashMap<String, Integer> ids;

    public PacketRegistry(PacketType... types) {
        this.reg = new HashMap<>(types.length);
        this.ids = new HashMap<>(types.length);
        int idx = 0;
        for (PacketType type : types) {
            reg.put(idx, type);
            ids.put(type.tag, idx);
            idx++;
        }
    }

    public PacketType getType(int id) {
        return reg.get(id);
    }

    public int getID(String tag) {
        return ids.get(tag);
    }
}
