package polyray;

import java.util.HashMap;
import java.util.Stack;

public class InputSystem {

    private static final Stack<BindSet> bindStack = new Stack<>();
    private static final HashMap<String, BindSet> bindSets = new HashMap<>();

    public static final BindSet getOrCreateBindSet(String name) {
        return bindSets.computeIfAbsent(name, k -> new BindSet(null));
    }

    public static final BindSet getOrCreateBindSet(String name, String deriveFrom) {
        BindSet derive = getOrCreateBindSet(deriveFrom);
        return bindSets.computeIfAbsent(name, k -> new BindSet(derive));
    }

    public static final void push(String name) {
        BindSet binds = bindSets.get(name);
        if(binds == null) {
            throw new IllegalArgumentException("Unknown BindSet: " + name);
        }
        bindStack.push(binds);
        binds.onPush();
    }

    public static final void pop() {
        BindSet binds = bindStack.pop();
        binds.onPop();
    }

    public static final void mousePress(float x, float y, int button) {
        bindStack.peek().mousePress(x, y, button);
    }

    public static final void keyPress(int key) {
        bindStack.peek().keyPress(key);
    }

    public static final boolean getKey(int key) {
        return bindStack.peek().getKey(key);
    }

    public static final boolean isCurrent(String name) {
        return bindStack.peek().equals(bindSets.get(name));
    }

    public static final boolean isActive(String name) {
        BindSet set = bindSets.get(name);
        for (BindSet b : bindStack) {
            if (set.equals(b)) {
                return true;
            }
        }
        return true;
    }
}
