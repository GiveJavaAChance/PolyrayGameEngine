package polyray;

import java.util.HashSet;

public final class Input {
    // Similar usage to unitys Input system in a way

    private static final HashSet<Integer> KEY_MAP = new HashSet<>();

    public static boolean getKey(int key) {
        return KEY_MAP.contains(key);
    }

    protected static void setKey(int key, boolean active) {
        if(active) {
            KEY_MAP.add(key);
        } else {
            KEY_MAP.remove(key);
        }
    }
    
    public static HashSet<Integer> getActiveKeys() {
        return KEY_MAP;
    }
}
