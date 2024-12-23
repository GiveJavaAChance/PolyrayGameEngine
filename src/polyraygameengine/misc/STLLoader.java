package polyraygameengine.misc;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

public class STLLoader {

    private final ArrayList<Triangle> triangles = new ArrayList<>();

    /**
     * Loads a stl model as triangles
     *
     * @param filepath The filepath to the stl file.
     * @param prec Should not be set to anything except 1. It is a niche way of
     * simplifying meshes by just removing triangles.
     * @param col The color of the triangle.
     * @param scale The scale of the model, how much it should be scaled. Note:
     * one centimeter in the model corresponds to a value of 100.0f.
     */
    public void loadSTLFile(String filepath, int prec, Vector3f color, float scale) {
        File file = new File(filepath);
        if (!file.exists()) {
            return;
        }
        try ( BufferedInputStream in = new BufferedInputStream(new FileInputStream(filepath))) {
            in.skipNBytes(96);
            int i = 0;
            while (in.available() > 0) {
                Vector3f[] v = new Vector3f[3];
                for (int j = 0; j < 3; j++) {
                    ByteBuffer buffer = ByteBuffer.wrap(in.readNBytes(12));
                    buffer.order(ByteOrder.LITTLE_ENDIAN);
                    float x = buffer.getFloat();
                    float y = buffer.getFloat();
                    float z = buffer.getFloat();
                    Vector3f vertex = new Vector3f(x, z, y); // The loaded triangles had swapped z and y so this is correcting that
                    v[j] = vertex;
                }
                if (i % prec == 0) {
                    Triangle t = new Triangle(v[0], v[1], v[2], color);
                    t = new Triangle(Vector3f.mul(t.a, scale), Vector3f.mul(t.b, scale), Vector3f.mul(t.c, scale), t.color);
                    triangles.add(t);
                }
                in.skipNBytes(14);
                i++;
            }
        } catch (IOException ex) {
        }
    }

    /**
     * Loads a stl model as triangles
     *
     * @param filepath The filepath to the stl file.
     * @param prec Should not be set to anything except 1. It is a niche way of
     * simplifying meshes by just removing triangles.
     * @param col The color of the triangle.
     * @param translate Translates the model around
     * @param scale The scale of the model, how much it should be scaled. Note:
     * one centimeter in the model corresponds to a value of 100.0f.
     */
    public void loadSTLFile(String filepath, int prec, Vector3f color, Vector3f translate, Vector3f scale) {
        try ( BufferedInputStream in = new BufferedInputStream(new FileInputStream(filepath))) {
            in.skipNBytes(96);
            int i = 0;
            while (in.available() > 0) {
                Vector3f[] v = new Vector3f[3];
                for (int j = 0; j < 3; j++) {
                    ByteBuffer buffer = ByteBuffer.wrap(in.readNBytes(12));
                    buffer.order(ByteOrder.LITTLE_ENDIAN);
                    float x = buffer.getFloat();
                    float y = buffer.getFloat();
                    float z = buffer.getFloat();
                    Vector3f vertex = new Vector3f(x, z, y); // The loaded triangles had swapped z and y so this is correcting that
                    v[j] = vertex;
                }
                if (i % prec == 0) {
                    for (int j = 0; j < 3; j++) {
                        v[j] = Vector3f.add(new Vector3f(v[j].x * scale.x, v[j].y * scale.y, v[j].z * scale.z), translate);
                    }
                    Triangle t = new Triangle(v[0], v[1], v[2], color);
                    triangles.add(t);
                }
                in.skipNBytes(14);
                i++;
            }
        } catch (IOException ex) {
        }
    }

    /**
     * Centers the loaded triangles, translating them won't make a difference.
     * centers by the bounding box, not by center of mass.
     */
    public void center() {
        Vector3f[] vertices = new Vector3f[triangles.size() * 3];
        for (int i = 0; i < triangles.size(); i++) {
            vertices[i * 3] = triangles.get(i).a;
            vertices[i * 3 + 1] = triangles.get(i).b;
            vertices[i * 3 + 2] = triangles.get(i).c;
        }
        float minX = vertices[0].x, maxX = vertices[0].x, minY = vertices[0].y, maxY = vertices[0].y, minZ = vertices[0].z, maxZ = vertices[0].z;
        for (Vector3f v : vertices) {
            if (v.x < minX) {
                minX = v.x;
            }
            if (v.x > maxX) {
                maxX = v.x;
            }
            if (v.y < minY) {
                minY = v.y;
            }
            if (v.y > maxY) {
                maxY = v.y;
            }
            if (v.z < minZ) {
                minZ = v.z;
            }
            if (v.z > maxZ) {
                maxZ = v.z;
            }
        }
        Vector3f minPos = new Vector3f(minX, minY, minZ);
        Vector3f maxPos = new Vector3f(maxX, maxY, maxZ);
        Vector3f center = Vector3f.mul(Vector3f.add(maxPos, minPos), 0.5f);
        for (Triangle t : triangles) {
            t.a.x -= center.x;
            t.a.y -= center.y;
            t.a.z -= center.z;
            t.b.x -= center.x;
            t.b.y -= center.y;
            t.b.z -= center.z;
            t.c.x -= center.x;
            t.c.y -= center.y;
            t.c.z -= center.z;
        }
    }

    /**
     * Retrieve triangles
     *
     * @return The loaded triangles
     */
    public ArrayList<Triangle> get() {
        return triangles;
    }

    /**
     * @return The amount of triangles loaded
     */
    public int getSize() {
        return triangles.size();
    }

    /**
     * Returns what file format it accepts, could be useful when dealing with
     * both OBJ and STL loaders.
     *
     * @return The File extension
     */
    public String getExtension() {
        return "stl";
    }
}
