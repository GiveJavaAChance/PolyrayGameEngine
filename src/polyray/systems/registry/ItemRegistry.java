package polyray.systems.item;

import java.util.HashMap;

public class ItemRegistry {

    private static final HashMap<String, ItemInfo> infos = new HashMap<>();

    public static final void register(ItemInfo itemInfo) {
        System.out.println("Item registered: " + itemInfo.name);
        infos.put(itemInfo.name, itemInfo);
    }

    public static final Item create(String name, int count) {
        return new Item(count, infos.get(name));
    }
}
