package polyray;

import java.util.HashMap;
import java.util.HashSet;

public class BindSet {

    private final BindSet parent;
    private final HashMap<Integer, MousePress> mouseBindings;
    private final HashMap<Integer, Runnable> keyBindings;
    private final HashSet<Integer> allowedKeys;
    private Runnable onPush, onPop;

    public BindSet(BindSet set) {
        this.parent = set;
        this.mouseBindings = new HashMap<>();
        this.keyBindings = new HashMap<>();
        this.allowedKeys = new HashSet<>();
    }

    public final void setOnPush(Runnable r) {
        this.onPush = r;
    }

    public final void setOnPop(Runnable r) {
        this.onPop = r;
    }

    public final void onPush() {
        if (parent != null) {
            parent.onPush();
        }
        if (onPush != null) {
            onPush.run();
        }
    }

    public final void onPop() {
        if (parent != null) {
            parent.onPop();
        }
        if (onPop != null) {
            onPop.run();
        }
    }

    public final void setMouseBinding(int button, MousePress r) {
        this.mouseBindings.put(button, r);
    }

    public final void setKeyBinding(int key, Runnable r) {
        this.keyBindings.put(key, r);
    }

    public final void mousePress(float x, float y, int button) {
        MousePress r = this.mouseBindings.get(button);
        if (r == null) {
            if (parent != null) {
                parent.mousePress(x, y, button);
            }
            return;
        }
        r.run(x, y);
    }

    public final void keyPress(int key) {
        Runnable r = this.keyBindings.get(key);
        if (r == null) {
            if (parent != null) {
                parent.keyPress(key);
            }
            return;
        }
        r.run();
    }

    public final void allowKeys(int... keys) {
        for (int key : keys) {
            this.allowedKeys.add(key);
        }
    }

    public final void blockKeys(int... keys) {
        for (int key : keys) {
            this.allowedKeys.remove(key);
        }
    }

    public final boolean getKey(int key) {
        return this.allowedKeys.contains(key) && Input.getKey(key);
    }

    @FunctionalInterface
    public interface MousePress {

        public void run(float x, float y);
    }
}
