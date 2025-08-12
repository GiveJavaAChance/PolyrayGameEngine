package polyray.objloader;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import javax.imageio.ImageIO;
import static org.lwjgl.opengl.GL11.GL_RGBA8;
import polyray.GLTexture;
import polyray.Material;
import polyray.ShaderProgram;
import polyray.Texture;
import polyray.TextureUtils;
import polyray.Vector2f;
import polyray.Vector3f;
import polyray.builtin.Instance3D;
import polyray.builtin.RenderObject;
import polyray.builtin.Vertex3D;

public class OBJLoader {

    public static ArrayList<RenderData> loadOBJFile(File objFile, File mtlFile) throws IOException {
        if (!objFile.exists() || !mtlFile.exists()) {
            return null;
        }

        ArrayList<Vector3f> v = new ArrayList<>();
        ArrayList<Vector3f> vn = new ArrayList<>();
        ArrayList<Vector2f> vt = new ArrayList<>();

        HashMap<String, OBJMaterial> materials = MTLParser.loadMTLFile(mtlFile);

        ArrayList<Vertex3D> vertices = null;
        String name = null;
        ArrayList<RenderData> objects = new ArrayList<>();

        BufferedReader reader = new BufferedReader(new FileReader(objFile));
        String line;
        while ((line = reader.readLine()) != null) {
            line = line.trim();
            if (line.startsWith("#") || line.isEmpty()) {
                continue;
            }
            String[] tokens = line.split("\\s+");
            if (tokens.length == 0) {
                continue;
            }
            switch (tokens[0]) {
                case "o" -> {
                    name = tokens[1];
                }
                case "v" -> {
                    v.add(new Vector3f(
                            Float.parseFloat(tokens[1]),
                            Float.parseFloat(tokens[2]),
                            Float.parseFloat(tokens[3])
                    ));
                }
                case "vt" -> {
                    vt.add(new Vector2f(
                            Float.parseFloat(tokens[1]),
                            1.0f - Float.parseFloat(tokens[2])
                    ));
                }
                case "vn" -> {
                    vn.add(new Vector3f(
                            Float.parseFloat(tokens[1]),
                            Float.parseFloat(tokens[2]),
                            Float.parseFloat(tokens[3])
                    ));
                }

                case "usemtl" -> {
                    vertices = new ArrayList<>();
                    objects.add(new RenderData(name, vertices, materials.get(tokens[1])));
                }
                case "f" -> {
                    addFace(tokens, vertices, v, vn, vt);
                }
            }
        }
        reader.close();
        return objects;
    }

    private static void addFace(String[] tokens, ArrayList<Vertex3D> vertices, ArrayList<Vector3f> v, ArrayList<Vector3f> vn, ArrayList<Vector2f> vt) {
        for (int i = 1; i <= 3; i++) {
            String[] indices = tokens[i].split("/");
            int vIdx = parseIndex(indices[0], v.size());
            int tIdx = (indices.length > 1 && !indices[1].isEmpty()) ? parseIndex(indices[1], vt.size()) : -1;
            int nIdx = (indices.length > 2) ? parseIndex(indices[2], vn.size()) : -1;

            Vector3f position = v.get(vIdx);
            Vector2f texCoord = (tIdx != -1) ? vt.get(tIdx) : new Vector2f();
            Vector3f normal = (nIdx != -1) ? vn.get(nIdx) : new Vector3f();

            vertices.add(new Vertex3D(
                    position.x, position.y, position.z,
                    normal.x, normal.y, normal.z,
                    texCoord.x, texCoord.y
            ));
        }
    }

    private static int parseIndex(String s, int size) {
        int index = Integer.parseInt(s);
        return (index < 0) ? (size + index) : (index - 1);
    }

    public static class RenderData {

        public final String name;
        public final ArrayList<Vertex3D> vertices;
        public final OBJMaterial mat;

        public RenderData(String name, ArrayList<Vertex3D> vertices, OBJMaterial mat) {
            this.name = name;
            this.vertices = vertices;
            this.mat = mat;
        }

        public RenderObject toDefault(int cameraBinding, int environmentBinding) throws IOException {
            Material m = new Material(cameraBinding, environmentBinding);
            OBJMaterial.toPBR(mat, m);
            return new RenderObject(new GLTexture(getTexture(), GL_RGBA8, true, true), m.getShader(), Vertex3D.VBO_TEMPLATE, Instance3D.VBO_TEMPLATE);
        }

        public Texture getTexture() throws IOException {
            if (mat.mapKd == null) {
                int r = (int) Math.max(Math.min(mat.Kd.x * 255.0f, 255.0f), 0.0f);
                int g = (int) Math.max(Math.min(mat.Kd.y * 255.0f, 255.0f), 0.0f);
                int b = (int) Math.max(Math.min(mat.Kd.z * 255.0f, 255.0f), 0.0f);
                return TextureUtils.createColorTexture(0xFF000000 | r << 16 | g << 8 | b);
            }
            return new Texture(ImageIO.read(new File(mat.mapKd)));
        }
    }
}
