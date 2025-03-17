package polyray;

import java.util.Iterator;
import polyray.builtin.Vertex3D;
import polyray.builtin.RenderObject;

public class SolidLoader {

    public static final void addCube(RenderObject obj) {
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, -0.5f, 0, 0, -1, 0, 0), // v0
                new Vertex3D(0.5f, -0.5f, -0.5f, 0, 0, -1, 1, 0), // v1
                new Vertex3D(0.5f, 0.5f, -0.5f, 0, 0, -1, 1, 1)); // v2
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, -0.5f, 0, 0, -1, 0, 0), // v0
                new Vertex3D(0.5f, 0.5f, -0.5f, 0, 0, -1, 1, 1), // v2
                new Vertex3D(-0.5f, 0.5f, -0.5f, 0, 0, -1, 0, 1)); // v3
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, 0.5f, 0, 0, 1, 0, 0), // v4
                new Vertex3D(0.5f, -0.5f, 0.5f, 0, 0, 1, 1, 0), // v5
                new Vertex3D(0.5f, 0.5f, 0.5f, 0, 0, 1, 1, 1)); // v6
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, 0.5f, 0, 0, 1, 0, 0), // v4
                new Vertex3D(0.5f, 0.5f, 0.5f, 0, 0, 1, 1, 1), // v6
                new Vertex3D(-0.5f, 0.5f, 0.5f, 0, 0, 1, 0, 1)); // v7
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, -0.5f, -1, 0, 0, 0, 0), // v0
                new Vertex3D(-0.5f, 0.5f, -0.5f, -1, 0, 0, 0, 1), // v3
                new Vertex3D(-0.5f, 0.5f, 0.5f, -1, 0, 0, 1, 1)); // v7
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, -0.5f, -1, 0, 0, 0, 0), // v0
                new Vertex3D(-0.5f, 0.5f, 0.5f, -1, 0, 0, 1, 1), // v7
                new Vertex3D(-0.5f, -0.5f, 0.5f, -1, 0, 0, 1, 0)); // v4
        obj.addTriangle(
                new Vertex3D(0.5f, -0.5f, -0.5f, 1, 0, 0, 0, 0), // v1
                new Vertex3D(0.5f, 0.5f, -0.5f, 1, 0, 0, 1, 0), // v2
                new Vertex3D(0.5f, 0.5f, 0.5f, 1, 0, 0, 1, 1)); // v6
        obj.addTriangle(
                new Vertex3D(0.5f, -0.5f, -0.5f, 1, 0, 0, 0, 0), // v1
                new Vertex3D(0.5f, 0.5f, 0.5f, 1, 0, 0, 1, 1), // v6
                new Vertex3D(0.5f, -0.5f, 0.5f, 1, 0, 0, 0, 1)); // v5
        obj.addTriangle(
                new Vertex3D(-0.5f, 0.5f, -0.5f, 0, 1, 0, 0, 0), // v3
                new Vertex3D(0.5f, 0.5f, -0.5f, 0, 1, 0, 1, 0), // v2
                new Vertex3D(0.5f, 0.5f, 0.5f, 0, 1, 0, 1, 1)); // v6
        obj.addTriangle(
                new Vertex3D(-0.5f, 0.5f, -0.5f, 0, 1, 0, 0, 0), // v3
                new Vertex3D(0.5f, 0.5f, 0.5f, 0, 1, 0, 1, 1), // v6
                new Vertex3D(-0.5f, 0.5f, 0.5f, 0, 1, 0, 0, 1)); // v7
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, -0.5f, 0, -1, 0, 0, 0), // v0
                new Vertex3D(0.5f, -0.5f, -0.5f, 0, -1, 0, 1, 0), // v1
                new Vertex3D(0.5f, -0.5f, 0.5f, 0, -1, 0, 1, 1)); // v5
        obj.addTriangle(
                new Vertex3D(-0.5f, -0.5f, -0.5f, 0, -1, 0, 0, 0), // v0
                new Vertex3D(0.5f, -0.5f, 0.5f, 0, -1, 0, 1, 1), // v5
                new Vertex3D(-0.5f, -0.5f, 0.5f, 0, -1, 0, 0, 1)); // v4
    }

    public static final void addBlock(RenderObject obj, float width, float height, float depth) {
        float hw = width * 0.5f;
        float hh = height * 0.5f;
        float hd = depth * 0.5f;
        obj.addTriangle(
                new Vertex3D(-hw, -hh, -hd, 0, 0, -1, 0, 0), // v0
                new Vertex3D(hw, -hh, -hd, 0, 0, -1, 1, 0), // v1
                new Vertex3D(hw, hh, -hd, 0, 0, -1, 1, 1)); // v2
        obj.addTriangle(
                new Vertex3D(-hw, -hh, -hd, 0, 0, -1, 0, 0), // v0
                new Vertex3D(hw, hh, -hd, 0, 0, -1, 1, 1), // v2
                new Vertex3D(-hw, hh, -hd, 0, 0, -1, 0, 1)); // v3
        obj.addTriangle(
                new Vertex3D(-hw, -hh, hd, 0, 0, 1, 0, 0), // v4
                new Vertex3D(hw, -hh, hd, 0, 0, 1, 1, 0), // v5
                new Vertex3D(hw, hh, hd, 0, 0, 1, 1, 1)); // v6
        obj.addTriangle(
                new Vertex3D(-hw, -hh, hd, 0, 0, 1, 0, 0), // v4
                new Vertex3D(hw, hh, hd, 0, 0, 1, 1, 1), // v6
                new Vertex3D(-hw, hh, hd, 0, 0, 1, 0, 1)); // v7
        obj.addTriangle(
                new Vertex3D(-hw, -hh, -hd, -1, 0, 0, 0, 0), // v0
                new Vertex3D(-hw, hh, -hd, -1, 0, 0, 0, 1), // v3
                new Vertex3D(-hw, hh, hd, -1, 0, 0, 1, 1)); // v7
        obj.addTriangle(
                new Vertex3D(-hw, -hh, -hd, -1, 0, 0, 0, 0), // v0
                new Vertex3D(-hw, hh, hd, -1, 0, 0, 1, 1), // v7
                new Vertex3D(-hw, -hh, hd, -1, 0, 0, 1, 0)); // v4
        obj.addTriangle(
                new Vertex3D(hw, -hh, -hd, 1, 0, 0, 0, 0), // v1
                new Vertex3D(hw, hh, -hd, 1, 0, 0, 1, 0), // v2
                new Vertex3D(hw, hh, hd, 1, 0, 0, 1, 1)); // v6
        obj.addTriangle(
                new Vertex3D(hw, -hh, -hd, 1, 0, 0, 0, 0), // v1
                new Vertex3D(hw, hh, hd, 1, 0, 0, 1, 1), // v6
                new Vertex3D(hw, -hh, hd, 1, 0, 0, 0, 1)); // v5
        obj.addTriangle(
                new Vertex3D(-hw, hh, -hd, 0, 1, 0, 0, 0), // v3
                new Vertex3D(hw, hh, -hd, 0, 1, 0, 1, 0), // v2
                new Vertex3D(hw, hh, hd, 0, 1, 0, 1, 1)); // v6
        obj.addTriangle(
                new Vertex3D(-hw, hh, -hd, 0, 1, 0, 0, 0), // v3
                new Vertex3D(hw, hh, hd, 0, 1, 0, 1, 1), // v6
                new Vertex3D(-hw, hh, hd, 0, 1, 0, 0, 1)); // v7
        obj.addTriangle(
                new Vertex3D(-hw, -hh, -hd, 0, -1, 0, 0, 0), // v0
                new Vertex3D(hw, -hh, -hd, 0, -1, 0, 1, 0), // v1
                new Vertex3D(hw, -hh, hd, 0, -1, 0, 1, 1)); // v5
        obj.addTriangle(
                new Vertex3D(-hw, -hh, -hd, 0, -1, 0, 0, 0), // v0
                new Vertex3D(hw, -hh, hd, 0, -1, 0, 1, 1), // v5
                new Vertex3D(-hw, -hh, hd, 0, -1, 0, 0, 1)); // v4
    }
    
    public static final void addIcoSphere(RenderObject obj, float radius, int detail) {
        IcoSphere ico = new IcoSphere(radius, detail);
        Iterator<Vertex3D> iter = ico.getVertices().iterator();
        while (iter.hasNext()) {
            obj.addTriangle(iter.next(), iter.next(), iter.next());
        }
    }
}
