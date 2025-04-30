package polyray.systems;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import polyray.Transform3D;
import polyray.Vector3f;
import polyray.builtin.Instance3D;
import polyray.modular.Instance;
import polyray.modular.RenderObjectBase;

public class Gizmos {

    private static final HashSet<Integer> update = new HashSet<>();
    private static final HashMap<Integer, RenderObjectBase> objects = new HashMap<>();
    private static final HashMap<Integer, Group> groups = new HashMap<>();
    private static final IDGenerator gen = new IDGenerator();

    public static final int newObject(RenderObjectBase obj) {
        int id = objects.size();
        objects.put(id, obj);
        return id;
    }

    public static final void update() {
        Iterator<Integer> iter = update.iterator();
        while (iter.hasNext()) {
            objects.get(iter.next()).uploadInstances();
            iter.remove();
        }
    }

    public static int pushLine(int type, Vector3f a, Vector3f b) {
        update.add(type);
        int id = gen.getNewID();
        groups.put(id, new Group(type, new Instance3D(getTransform(a, b))));
        return id;
    }

    public static int pushLineStrip(int type, Vector3f... vertices) {
        if (vertices.length < 2) {
            return -1;
        }
        update.add(type);
        Instance3D[] inst = new Instance3D[vertices.length - 1];
        Vector3f prev = vertices[0];
        for (int i = 1; i < vertices.length; i++) {
            Vector3f pos = vertices[i];
            inst[i - 1] = new Instance3D(getTransform(prev, pos));
            prev = pos;
        }
        int id = gen.getNewID();
        groups.put(id, new Group(type, inst));
        return id;
    }

    public static int pushPoints(int type, Vector3f... pos) {
        update.add(type);
        Instance3D[] inst = new Instance3D[pos.length];
        for (int i = 0; i < pos.length; i++) {
            Transform3D tx = new Transform3D();
            Vector3f p = pos[i];
            tx.translate(p.x, p.y, p.z);
            inst[i] = new Instance3D(tx);
        }
        int id = gen.getNewID();
        groups.put(id, new Group(type, inst));
        return id;
    }

    public static final void pop(int id) {
        if (groups.isEmpty()) {
            return;
        }
        Group l = groups.remove(id);
        if (l != null) {
            l.remove();
            gen.freeID(id);
        }
    }

    private static Transform3D getTransform(Vector3f a, Vector3f b) {
        Vector3f N = Vector3f.sub(b, a);
        Vector3f T = new Vector3f();
        Vector3f B = new Vector3f();
        TBN(N, T, B);
        Transform3D tx = new Transform3D();
        tx.setToAxisTransform(T, N, B, a);
        return tx;
    }

    private static void TBN(Vector3f N, Vector3f T, Vector3f B) {
        Vector3f ref;
        if (Math.abs(N.x) < Math.abs(N.y) && Math.abs(N.x) < Math.abs(N.z)) {
            ref = new Vector3f(1.0f, 0, 0);
        } else if (Math.abs(N.y) < Math.abs(N.z)) {
            ref = new Vector3f(0, 1.0f, 0);
        } else {
            ref = new Vector3f(0, 0, 1.0f);
        }
        Vector3f t = Vector3f.normalize(Vector3f.cross(ref, N));
        Vector3f b = Vector3f.normalize(Vector3f.cross(N, t));
        T.x = t.x;
        T.y = t.y;
        T.z = t.z;
        B.x = b.x;
        B.y = b.y;
        B.z = b.z;
    }

    private static class Group {

        private final int type;
        public final Instance[] instances;

        public Group(int type, Instance... lines) {
            this.type = type;
            this.instances = lines;
            RenderObjectBase obj = objects.get(type);
            for (Instance i : lines) {
                obj.addInstance(i);
            }
        }

        public void remove() {
            RenderObjectBase obj = objects.get(type);
            for (Instance i : instances) {
                obj.removeInstance(i);
            }
        }
    }
}
