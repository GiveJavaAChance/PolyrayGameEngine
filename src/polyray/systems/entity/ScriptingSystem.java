package polyray.systems.entity;

import java.util.ArrayList;

public class ScriptingSystem {

    private static final ArrayList<Script> scripts = new ArrayList<>();
    
    public static final void addEntity(Entity e) {
        ArrayList<Script> sc = new ArrayList<>();
        e.getComponents(Script.class, sc);
        for (Script s : sc) {
            s.parent = e;
            s.setup();
        }
        scripts.addAll(sc);
    }
    
    public static final void removeEntity(Entity e) {
        ArrayList<Script> sc = new ArrayList<>();
        e.getComponents(Script.class, sc);
        for (Script s : sc) {
            s.parent = null;
            scripts.remove(s);
        }
    }
    
    public static final void frameUpdate(double dt) {
        for (Script s : scripts) {
            s.frameUpdate(dt);
        }
    }
    
    public static final void physicsUpdate(double dt) {
        for (Script s : scripts) {
            s.physicsUpdate(dt);
        }
    }
}
