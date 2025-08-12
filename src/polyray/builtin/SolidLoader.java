package polyray.builtin;

import java.util.Collection;
import polyray.IcoSphere;

public class SolidLoader {

    public static final void addCube(Collection<Vertex3D> obj) {
        addBlock(obj, 1.0f, 1.0f, 1.0f);
    }

    public static final void addBlock(Collection<Vertex3D> obj, float width, float height, float depth) {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        float hd = depth * 0.5f;

        obj.add(new Vertex3D(-hw, -hh, -hd, 0, 0, -1, 0, 0)); // v0
        obj.add(new Vertex3D(hw, -hh, -hd, 0, 0, -1, 1, 0)); // v1
        obj.add(new Vertex3D(hw, hh, -hd, 0, 0, -1, 1, 1)); // v2

        obj.add(new Vertex3D(-hw, -hh, -hd, 0, 0, -1, 0, 0)); // v0
        obj.add(new Vertex3D(hw, hh, -hd, 0, 0, -1, 1, 1)); // v2
        obj.add(new Vertex3D(-hw, hh, -hd, 0, 0, -1, 0, 1)); // v3

        obj.add(new Vertex3D(-hw, -hh, hd, 0, 0, 1, 0, 0)); // v4
        obj.add(new Vertex3D(hw, -hh, hd, 0, 0, 1, 1, 0)); // v5
        obj.add(new Vertex3D(hw, hh, hd, 0, 0, 1, 1, 1)); // v6

        obj.add(new Vertex3D(-hw, -hh, hd, 0, 0, 1, 0, 0)); // v4
        obj.add(new Vertex3D(hw, hh, hd, 0, 0, 1, 1, 1)); // v6
        obj.add(new Vertex3D(-hw, hh, hd, 0, 0, 1, 0, 1)); // v7

        obj.add(new Vertex3D(-hw, -hh, -hd, -1, 0, 0, 0, 0)); // v0
        obj.add(new Vertex3D(-hw, hh, -hd, -1, 0, 0, 0, 1)); // v3
        obj.add(new Vertex3D(-hw, hh, hd, -1, 0, 0, 1, 1)); // v7

        obj.add(new Vertex3D(-hw, -hh, -hd, -1, 0, 0, 0, 0)); // v0
        obj.add(new Vertex3D(-hw, hh, hd, -1, 0, 0, 1, 1)); // v7
        obj.add(new Vertex3D(-hw, -hh, hd, -1, 0, 0, 1, 0)); // v4

        obj.add(new Vertex3D(hw, -hh, -hd, 1, 0, 0, 0, 0)); // v1
        obj.add(new Vertex3D(hw, hh, -hd, 1, 0, 0, 1, 0)); // v2
        obj.add(new Vertex3D(hw, hh, hd, 1, 0, 0, 1, 1)); // v6

        obj.add(new Vertex3D(hw, -hh, -hd, 1, 0, 0, 0, 0)); // v1
        obj.add(new Vertex3D(hw, hh, hd, 1, 0, 0, 1, 1)); // v6
        obj.add(new Vertex3D(hw, -hh, hd, 1, 0, 0, 0, 1)); // v5

        obj.add(new Vertex3D(-hw, hh, -hd, 0, 1, 0, 0, 0)); // v3
        obj.add(new Vertex3D(hw, hh, -hd, 0, 1, 0, 1, 0)); // v2
        obj.add(new Vertex3D(hw, hh, hd, 0, 1, 0, 1, 1)); // v6

        obj.add(new Vertex3D(-hw, hh, -hd, 0, 1, 0, 0, 0)); // v3
        obj.add(new Vertex3D(hw, hh, hd, 0, 1, 0, 1, 1)); // v6
        obj.add(new Vertex3D(-hw, hh, hd, 0, 1, 0, 0, 1)); // v7

        obj.add(new Vertex3D(-hw, -hh, -hd, 0, -1, 0, 0, 0)); // v0
        obj.add(new Vertex3D(hw, -hh, -hd, 0, -1, 0, 1, 0)); // v1
        obj.add(new Vertex3D(hw, -hh, hd, 0, -1, 0, 1, 1)); // v5

        obj.add(new Vertex3D(-hw, -hh, -hd, 0, -1, 0, 0, 0)); // v0
        obj.add(new Vertex3D(hw, -hh, hd, 0, -1, 0, 1, 1)); // v5
        obj.add(new Vertex3D(-hw, -hh, hd, 0, -1, 0, 0, 1)); // v4
    }

    public static final void addIcoSphere(Collection<Vertex3D> obj, float radius, int detail) {
        for (Vertex3D v : new IcoSphere(radius, detail).getVertices()) {
            obj.add(v);
        }
    }
}
