package polyray.systems.loader;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.Stack;
import javax.imageio.ImageIO;
import static org.lwjgl.opengl.GL11.GL_RGBA8;
import polyray.GLTexture;
import polyray.Material;
import polyray.ResourceLoader;
import polyray.Texture;
import polyray.TextureUtils;
import polyray.Vector3f;
import polyray.builtin.Instance3D;
import polyray.builtin.RenderObject;
import polyray.builtin.Vertex3D;
import polyray.modular.Instance;

public class MeshLoader {

    private static final Stack<MeshInfo> meshes = new Stack<>();

    public static final void addMesh(float c, String... names) {
        meshes.push(new MeshInfo(c, names));
    }

    public static final void addMesh(float r, float g, float b, String... names) {
        meshes.push(new MeshInfo(r, g, b, names));
    }

    public static final void addMesh(float r, float g, float b, float a, String... names) {
        meshes.push(new MeshInfo(r, g, b, a, names));
    }

    public static final void addMesh(String texName, Vector3f T, Vector3f B, String... names) {
        meshes.push(new MeshInfo(texName, T, B, names));
    }

    public static final RenderObject[] load(Material mat) {
        RenderObject[] objects = new RenderObject[meshes.size()];
        for (int i = 0; i < objects.length; i++) {
            MeshInfo info = meshes.pop();
            GLTexture texture;
            if (info.tex != null) {
                try {
                    texture = new GLTexture(new Texture(ImageIO.read(ResourceLoader.getLoader().getResourceAsStream(info.tex.name))), GL_RGBA8, false, true);
                } catch (IOException e) {
                    texture = new GLTexture(TextureUtils.createColorTexture(0xFFFFFFFF), GL_RGBA8, false, false);
                }
            } else {
                texture = new GLTexture(TextureUtils.createColorTexture(col(info.r, info.g, info.b, info.a)), GL_RGBA8, false, false);
            }
            RenderObject<Vertex3D, Instance> obj = new RenderObject<>(texture, mat.getShader(), Vertex3D.VBO_TEMPLATE, Instance3D.VBO_TEMPLATE);
            if (info.tex != null) {
                for (String name : info.names) {
                    try {
                        loadMeshTex(name, info.tex.T, info.tex.B, obj.vertices);
                    } catch (IOException e) {
                    }
                }
            } else {
                for (String name : info.names) {
                    try {
                        loadMesh(name, obj.vertices);
                    } catch (IOException e) {
                    }
                }
            }
            obj.upload();
            objects[i] = obj;
        }
        return objects;
    }

    private static int col(float r, float g, float b, float a) {
        int R = (int) (r * 256.0f);
        int G = (int) (g * 256.0f);
        int B = (int) (b * 256.0f);
        int A = (int) (a * 256.0f);
        return clamp(A) << 24 | clamp(R) << 16 | clamp(G) << 8 | clamp(B);
    }

    private static int clamp(int i) {
        if (i < 0) {
            return 0;
        }
        if (i > 255) {
            return 255;
        }
        return i;
    }

    public static final void loadMesh(String meshName, ArrayList<Vertex3D> verts) throws IOException {
        BufferedInputStream in = new BufferedInputStream(ResourceLoader.getLoader().getResourceAsStream(meshName));
        byte[] buffer = new byte[36];
        float[] vertices = new float[9];
        FloatBuffer buff = ByteBuffer.wrap(buffer).asFloatBuffer();
        while (in.read(buffer) == 36) {
            buff.get(0, vertices);
            Vertex3D a = new Vertex3D(vertices[0], vertices[2], -vertices[1], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            Vertex3D b = new Vertex3D(vertices[3], vertices[5], -vertices[4], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            Vertex3D c = new Vertex3D(vertices[6], vertices[8], -vertices[7], 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
            alignNormals(a, b, c);
            verts.add(a);
            verts.add(b);
            verts.add(c);
        }
    }

    public static final void loadMeshTex(String meshName, Vector3f T, Vector3f B, ArrayList<Vertex3D> verts) throws IOException {
        BufferedInputStream in = new BufferedInputStream(ResourceLoader.getLoader().getResourceAsStream(meshName));
        byte[] buffer = new byte[36];
        float[] vertices = new float[9];
        FloatBuffer buff = ByteBuffer.wrap(buffer).asFloatBuffer();
        while (in.read(buffer) == 36) {
            buff.get(0, vertices);
            Vector3f av = new Vector3f(vertices[0], vertices[2], -vertices[1]);
            Vector3f bv = new Vector3f(vertices[3], vertices[5], -vertices[4]);
            Vector3f cv = new Vector3f(vertices[6], vertices[8], -vertices[7]);
            Vertex3D a = new Vertex3D(av.x, av.y, av.z, 0.0f, 0.0f, 0.0f, Vector3f.dot(T, av), Vector3f.dot(B, av));
            Vertex3D b = new Vertex3D(bv.x, bv.y, bv.z, 0.0f, 0.0f, 0.0f, Vector3f.dot(T, bv), Vector3f.dot(B, bv));
            Vertex3D c = new Vertex3D(cv.x, cv.y, cv.z, 0.0f, 0.0f, 0.0f, Vector3f.dot(T, cv), Vector3f.dot(B, cv));
            alignNormals(a, b, c);
            verts.add(a);
            verts.add(b);
            verts.add(c);
        }
    }

    private static void alignNormals(Vertex3D a, Vertex3D b, Vertex3D c) {
        Vector3f normal = getNormal(
                new Vector3f(a.x, a.y, a.z),
                new Vector3f(b.x, b.y, b.z),
                new Vector3f(c.x, c.y, c.z)
        );
        a.nx = normal.x;
        a.ny = normal.y;
        a.nz = normal.z;
        b.nx = normal.x;
        b.ny = normal.y;
        b.nz = normal.z;
        c.nx = normal.x;
        c.ny = normal.y;
        c.nz = normal.z;
    }

    private static Vector3f getNormal(Vector3f a, Vector3f b, Vector3f c) {
        Vector3f u = Vector3f.sub(b, a);
        Vector3f v = Vector3f.sub(c, a);
        return Vector3f.cross(u, v);
    }

    private static class MeshInfo {

        public final float r, g, b, a;
        public final String[] names;
        public final TexInfo tex;

        public MeshInfo(float c, String... names) {
            this.r = c;
            this.g = c;
            this.b = c;
            this.a = 1.0f;
            this.names = names;
            this.tex = null;
        }

        public MeshInfo(float r, float g, float b, String... names) {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = 1.0f;
            this.names = names;
            this.tex = null;
        }

        public MeshInfo(float r, float g, float b, float a, String... names) {
            this.r = r;
            this.g = g;
            this.b = b;
            this.a = a;
            this.names = names;
            this.tex = null;
        }

        public MeshInfo(String texName, Vector3f T, Vector3f B, String... names) {
            this.r = 0.0f;
            this.g = 0.0f;
            this.b = 0.0f;
            this.a = 0.0f;
            this.names = names;
            this.tex = new TexInfo(T, B, texName);
        }
    }

    private static class TexInfo {

        public final String name;
        public final Vector3f T, B;

        public TexInfo(Vector3f T, Vector3f B, String name) {
            this.name = name;
            this.T = T;
            this.B = B;
        }
    }
}
