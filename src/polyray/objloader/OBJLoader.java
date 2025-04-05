package polyray.objloader;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import javax.imageio.ImageIO;
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

    public static ArrayList<RenderObject> loadOBJFile(File objFile, File mtlFile, int cameraBinding, int environmentBinding) throws IOException {
        return loadOBJFile(objFile, mtlFile, () -> {
            return new Material(cameraBinding, environmentBinding).getShader();
        });
    }

    public static ArrayList<RenderObject> loadOBJFile(File objFile, File mtlFile, ShaderBuilder builder) throws IOException {
        if (!objFile.exists() || !mtlFile.exists()) {
            return null;
        }

        ArrayList<Vector3f> v = new ArrayList<>();
        ArrayList<Vector3f> vn = new ArrayList<>();
        ArrayList<Vector2f> vt = new ArrayList<>();

        HashMap<String, OBJMaterial> materials = MTLParser.loadMTLFile(mtlFile);

        ArrayList<RenderObject> objects = new ArrayList<>();
        RenderObject currentObject = null;

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
                    OBJMaterial m = materials.get(tokens[1]);
                    Material mat = new Material(builder.build());
                    /*mat.setRoughness(0.5f);
                    mat.setMetallic(0.5f);
                    mat.setF0(new Vector3f(0.05f, 0.05f, 0.05f));*/
                    OBJMaterial.toPBR(m, mat);
                    if (m.mapKd == null) {
                        int r = (int) Math.max(Math.min(m.Kd.x * 255.0f, 255.0f), 0.0f);
                        int g = (int) Math.max(Math.min(m.Kd.y * 255.0f, 255.0f), 0.0f);
                        int b = (int) Math.max(Math.min(m.Kd.z * 255.0f, 255.0f), 0.0f);
                        currentObject = new RenderObject(TextureUtils.createColorTexture(0xFF000000 | r << 16 | g << 8 | b), mat.getShader(), Vertex3D.VBO_TEMPLATE, Instance3D.VBO_TEMPLATE);
                    } else {
                        Texture tex = new Texture(ImageIO.read(new File(m.mapKd)));
                        currentObject = new RenderObject(tex, mat.getShader(), Vertex3D.VBO_TEMPLATE, Instance3D.VBO_TEMPLATE);
                    }
                    objects.add(currentObject);
                }
                case "f" -> {
                    addFace(tokens, currentObject, v, vn, vt);
                }
            }
        }
        reader.close();
        return objects;
    }

    private static void addFace(String[] tokens, RenderObject obj, ArrayList<Vector3f> v, ArrayList<Vector3f> vn, ArrayList<Vector2f> vt) {
        for (int i = 1; i <= 3; i++) {
            String[] indices = tokens[i].split("/");
            int vIdx = parseIndex(indices[0], v.size());
            int tIdx = (indices.length > 1 && !indices[1].isEmpty()) ? parseIndex(indices[1], vt.size()) : -1;
            int nIdx = (indices.length > 2) ? parseIndex(indices[2], vn.size()) : -1;

            Vector3f position = v.get(vIdx);
            Vector2f texCoord = (tIdx != -1) ? vt.get(tIdx) : new Vector2f();
            Vector3f normal = (nIdx != -1) ? vn.get(nIdx) : new Vector3f();

            obj.addVertex(new Vertex3D(
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

    public static interface ShaderBuilder {

        public ShaderProgram build();
    }
}
