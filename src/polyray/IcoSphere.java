package polyray;

import java.util.ArrayList;
import polyray.builtin.Vertex3D;

public class IcoSphere {

    private final ArrayList<Vector3f> vertices;
    private final ArrayList<Triangle> triangles;
    private final float radius;
    private final int detail;

    public IcoSphere(float radius, int detail) {
        this.vertices = new ArrayList<>();
        this.triangles = new ArrayList<>();
        this.radius = radius;
        this.detail = detail;
        generateIcoSphere();
    }

    private void generateIcoSphere() {
        float t = (float) ((1.0f + Math.sqrt(5.0f)) / 2.0f);
        addVertex(-1.0f, t, 0.0f);
        addVertex(1.0f, t, 0.0f);
        addVertex(-1.0f, -t, 0.0f);
        addVertex(1.0f, -t, 0.0f);
        addVertex(0.0f, -1.0f, t);
        addVertex(0.0f, 1.0f, t);
        addVertex(0.0f, -1.0f, -t);
        addVertex(0.0f, 1.0f, -t);
        addVertex(t, 0.0f, -1.0f);
        addVertex(t, 0.0f, 1.0f);
        addVertex(-t, 0.0f, -1.0f);
        addVertex(-t, 0.0f, 1.0f);

        ArrayList<Triangle> tris = new ArrayList<>();
        tris.add(new Triangle(vertices.get(0), vertices.get(11), vertices.get(5)));
        tris.add(new Triangle(vertices.get(0), vertices.get(5), vertices.get(1)));
        tris.add(new Triangle(vertices.get(0), vertices.get(1), vertices.get(7)));
        tris.add(new Triangle(vertices.get(0), vertices.get(7), vertices.get(10)));
        tris.add(new Triangle(vertices.get(0), vertices.get(10), vertices.get(11)));

        tris.add(new Triangle(vertices.get(1), vertices.get(5), vertices.get(9)));
        tris.add(new Triangle(vertices.get(5), vertices.get(11), vertices.get(4)));
        tris.add(new Triangle(vertices.get(11), vertices.get(10), vertices.get(2)));
        tris.add(new Triangle(vertices.get(10), vertices.get(7), vertices.get(6)));
        tris.add(new Triangle(vertices.get(7), vertices.get(1), vertices.get(8)));

        tris.add(new Triangle(vertices.get(3), vertices.get(9), vertices.get(4)));
        tris.add(new Triangle(vertices.get(3), vertices.get(4), vertices.get(2)));
        tris.add(new Triangle(vertices.get(3), vertices.get(2), vertices.get(6)));
        tris.add(new Triangle(vertices.get(3), vertices.get(6), vertices.get(8)));
        tris.add(new Triangle(vertices.get(3), vertices.get(8), vertices.get(9)));

        tris.add(new Triangle(vertices.get(4), vertices.get(9), vertices.get(5)));
        tris.add(new Triangle(vertices.get(2), vertices.get(4), vertices.get(11)));
        tris.add(new Triangle(vertices.get(6), vertices.get(2), vertices.get(10)));
        tris.add(new Triangle(vertices.get(8), vertices.get(6), vertices.get(7)));
        tris.add(new Triangle(vertices.get(9), vertices.get(8), vertices.get(1)));

        for (int i = 0; i < detail; i++) {
            ArrayList<Triangle> newFaces = new ArrayList<>();
            for (Triangle tri : tris) {
                Vector3f a = getMiddlePoint(tri.a, tri.b);
                Vector3f b = getMiddlePoint(tri.b, tri.c);
                Vector3f c = getMiddlePoint(tri.c, tri.a);
                newFaces.add(new Triangle(tri.a, a, c));
                newFaces.add(new Triangle(tri.b, b, a));
                newFaces.add(new Triangle(tri.c, c, b));
                newFaces.add(new Triangle(a, b, c));
            }
            tris = newFaces;
        }
        this.triangles.addAll(tris);
    }

    private void addVertex(float x, float y, float z) {
        this.vertices.add(Vector3f.mul(Vector3f.normalize(new Vector3f(x, y, z)), radius));
    }

    private Vector3f getMiddlePoint(Vector3f p1, Vector3f p2) {
        return Vector3f.mul(Vector3f.normalize(Vector3f.add(p1, p2)), radius);
    }

    public ArrayList<Vertex3D> getVertices() {
        ArrayList<Vertex3D> verts = new ArrayList<>(triangles.size() * 3);
        for (Triangle t : triangles) {
            Vector3f a = t.a;
            Vector3f b = t.b;
            Vector3f c = t.c;
            verts.add(new Vertex3D(a.x, a.y, a.z, a.x / radius, a.y / radius, a.z / radius, a.x / radius * 0.5f + 0.5f, a.z / radius * 0.5f + 0.5f));
            verts.add(new Vertex3D(b.x, b.y, b.z, b.x / radius, b.y / radius, b.z / radius, b.x / radius * 0.5f + 0.5f, b.z / radius * 0.5f + 0.5f));
            verts.add(new Vertex3D(c.x, c.y, c.z, c.x / radius, c.y / radius, c.z / radius, c.x / radius * 0.5f + 0.5f, c.z / radius * 0.5f + 0.5f));
        }
        return verts;
    }

    private static class Triangle {

        public Vector3f a, b, c;

        public Triangle(Vector3f a, Vector3f b, Vector3f c) {
            this.a = a;
            this.b = b;
            this.c = c;
        }
    }
}
