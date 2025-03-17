package polyray;

import java.util.HashMap;

public final class Input {
    // Similar usage to unitys Input system in a way

    private static final HashMap<Integer, Boolean> KEY_MAP = new HashMap<>();

    public static boolean getKey(int key) {
        Boolean b = KEY_MAP.get(key);
        if (b == null) {
            return false;
        }
        return b;
    }

    protected static void setKey(int key, boolean active) {
        KEY_MAP.put(key, active);
    }
}
