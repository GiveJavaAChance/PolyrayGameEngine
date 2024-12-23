package polyraygameengine.misc;

import java.awt.Color;
import java.awt.image.BufferedImage;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import javax.imageio.ImageIO;

public class OBJLoader {

    private final ArrayList<Vector3f> vertices = new ArrayList<>();
    private final ArrayList<Triangle> triangles = new ArrayList<>();
    private final ArrayList<ArrayList<Triangle>> objects = new ArrayList<>();
    private final ArrayList<String> objectNames = new ArrayList<>();
    private final ArrayList<Vector2f> colorPos = new ArrayList<>();
    private final ArrayList<Vector3f> normals = new ArrayList<>();

    /**
     * Loads a obj file as triangles
     *
     * @param file
     */
    public void loadOBJFile(File file) {
        if (!file.exists()) {
            return;
        }
        try {
            String filePath = file.getAbsolutePath();
            String mtlFilePath = filePath.replace("obj", "mtl");
            String folder = file.getParentFile().getAbsolutePath();
            String texturePath = findTexturesDirectory(folder);
            FileReader fileReader = new FileReader(filePath);
            BufferedReader bufferedReader = new BufferedReader(fileReader);
            FileReader mtlFileReader = new FileReader(mtlFilePath);
            BufferedReader mtlBufferedReader = new BufferedReader(mtlFileReader);
            String line;
            ArrayList<String> filepaths = new ArrayList<>();
            String name = "";
            while ((line = mtlBufferedReader.readLine()) != null) {
                if (line.startsWith("newmtl ")) {
                    name = line.replace("newmtl ", "").trim().toLowerCase();
                }
                if (line.startsWith("map_Kd ")) {
                    String filepath = line.replace("map_Kd ", "");
                    File filep = new File(filepath);
                    String fileName = filep.getName();
                    filepaths.add(name + " " + fileName.trim());
                }
            }
            BufferedImage image = null;
            int width = 0, height = 0;
            ArrayList<Triangle> objectTriangles = new ArrayList<>();
            while ((line = bufferedReader.readLine()) != null) {
                if (line.startsWith("v ")) {
                    String data = line.replace("v ", "");
                    int separator1 = data.indexOf(" ");
                    String x = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String y = data.substring(separator1, separator2).trim();
                    String z = data.substring(separator2).trim();
                    float X = Float.parseFloat(x);
                    float Y = Float.parseFloat(y);
                    float Z = -Float.parseFloat(z);
                    Vector3f v = new Vector3f(X, Y, Z);
                    vertices.add(v);
                } else if (line.startsWith("vt ")) {
                    String data = line.replace("vt ", "");
                    int separator1 = data.indexOf(" ");
                    String a = data.substring(0, separator1).trim();
                    String b = data.substring(separator1).trim();
                    float x = Float.parseFloat(a);
                    float y = Float.parseFloat(b);
                    Vector2f v = new Vector2f(x, y);
                    colorPos.add(v);
                } else if (line.startsWith("f ")) {
                    String data = line.replace("f ", "");
                    int separator1 = data.indexOf(" ");
                    String a = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String b = data.substring(separator1, separator2).trim();
                    String c = data.substring(separator2).trim();
                    // Indexes
                    int vIndexA = a.indexOf("/");
                    int vIndexB = b.indexOf("/");
                    int vIndexC = c.indexOf("/");
                    String aVIdx = a.substring(0, vIndexA);
                    String bVIdx = b.substring(0, vIndexB);
                    String cVIdx = c.substring(0, vIndexC);
                    int vAi = Integer.parseInt(aVIdx) - 1;
                    int vBi = Integer.parseInt(bVIdx) - 1;
                    int vCi = Integer.parseInt(cVIdx) - 1;
                    if (line.contains("//")) {
                        Triangle t = new Triangle(vertices.get(vAi), vertices.get(vBi), vertices.get(vCi), new Vector3f(1.0f));
                        triangles.add(t);
                        objectTriangles.add(t);
                        continue;
                    }

                    int colA = a.indexOf("/", vIndexA + 1);
                    int colB = b.indexOf("/", vIndexB + 1);
                    int colC = c.indexOf("/", vIndexC + 1);
                    // Parsing
                    String icolA = a.substring(vIndexA + 1, colA);
                    String icolB = b.substring(vIndexB + 1, colB);
                    String icolC = c.substring(vIndexC + 1, colC);
                    int aColI = Integer.parseInt(icolA) - 1;
                    int bColI = Integer.parseInt(icolB) - 1;
                    int cColI = Integer.parseInt(icolC) - 1;
                    Vector2f posA = colorPos.get(aColI);
                    Vector2f posB = colorPos.get(bColI);
                    Vector2f posC = colorPos.get(cColI);
                    Vector3f triangleCol = new Vector3f(1.0f);
                    if (image != null) {
                        Vector3f cA = toColor(image.getRGB((int) (posA.x * (width - 1.0f)), (int) ((1.0f - posA.y) * (height - 1.0f))));
                        Vector3f cB = toColor(image.getRGB((int) (posB.x * (width - 1.0f)), (int) ((1.0f - posB.y) * (height - 1.0f))));
                        Vector3f cC = toColor(image.getRGB((int) (posC.x * (width - 1.0f)), (int) ((1.0f - posC.y) * (height - 1.0f))));
                        triangleCol = Vector3f.mul(Vector3f.add(Vector3f.add(cA, cB), cC), 1.0f / 3.0f);
                    }
                    Triangle t = new Triangle(vertices.get(vAi), vertices.get(vBi), vertices.get(vCi), triangleCol);
                    triangles.add(t);
                    objectTriangles.add(t);
                } else if (line.startsWith("vn ")) {
                    String data = line.replace("vn ", "");
                    int separator1 = data.indexOf(" ");
                    String x = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String y = data.substring(separator1, separator2).trim();
                    String z = data.substring(separator2).trim();
                    float X = Float.parseFloat(x);
                    float Y = Float.parseFloat(y);
                    float Z = Float.parseFloat(z);
                    Vector3f v = new Vector3f(X, Y, Z);
                    normals.add(v);
                } else if (line.startsWith("o ")) {
                    String data = line.replace("o ", "").trim().toLowerCase();
                    if (!objectTriangles.isEmpty()) {
                        objects.add(objectTriangles);
                        objectNames.add(data);
                        objectTriangles.clear();
                    }
                    for (String filepath : filepaths) {
                        String objName = filepath.substring(0, filepath.indexOf(" ")).trim();
                        String fileName = filepath.substring(filepath.indexOf(" ") + 1).trim();
                        if (data.contains(objName)) {
                            File f = new File(texturePath + "\\" + fileName);
                            if (f.exists()) {
                                image = ImageIO.read(f);
                                width = image.getWidth();
                                height = image.getHeight();
                            }
                        }
                    }
                }
            }
        } catch (IOException e) {
        }
        colorPos.clear();
    }

    public void loadOBJFile(File file, Vector3f translate, Vector3f scale) {
        if (!file.exists()) {
            return;
        }
        try {
            String filePath = file.getAbsolutePath();
            String mtlFilePath = filePath.replace("obj", "mtl");
            String folder = file.getParentFile().getAbsolutePath();
            String texturePath = findTexturesDirectory(folder);
            FileReader fileReader = new FileReader(filePath);
            BufferedReader bufferedReader = new BufferedReader(fileReader);
            FileReader mtlFileReader = new FileReader(mtlFilePath);
            BufferedReader mtlBufferedReader = new BufferedReader(mtlFileReader);
            String line;
            ArrayList<String> filepaths = new ArrayList<>();
            ArrayList<Vector3f> cols = new ArrayList<>();
            ArrayList<String> materialColorNames = new ArrayList<>();
            String name = "";
            BufferedImage image = null;
            Vector3f currCol = null;
            while ((line = mtlBufferedReader.readLine()) != null) {
                if (line.startsWith("newmtl ")) {
                    name = line.replace("newmtl ", "").trim().toLowerCase();
                }
                if (line.startsWith("map_Kd ")) {
                    String filepath = line.replace("map_Kd ", "");
                    File filep = new File(filepath);
                    String fileName = filep.getName();
                    filepaths.add(name + " " + fileName.trim());
                }
                if (line.startsWith("Kd ")) {
                    String data = line.replace("Kd ", "");
                    int separator1 = data.indexOf(" ");
                    String r = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String g = data.substring(separator1, separator2).trim();
                    String b = data.substring(separator2).trim();
                    float R = Float.parseFloat(r);
                    float G = Float.parseFloat(g);
                    float B = -Float.parseFloat(b);
                    Vector3f col = new Vector3f(R, G, B);
                    cols.add(col);
                    materialColorNames.add(name);
                }
            }

            int width = 0, height = 0;
            ArrayList<Triangle> objectTriangles = new ArrayList<>();
            while ((line = bufferedReader.readLine()) != null) {
                if (line.startsWith("v ")) {
                    String data = line.replace("v ", "");
                    int separator1 = data.indexOf(" ");
                    String x = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String y = data.substring(separator1, separator2).trim();
                    String z = data.substring(separator2).trim();
                    float X = Float.parseFloat(x);
                    float Y = Float.parseFloat(y);
                    float Z = -Float.parseFloat(z);
                    Vector3f v = Vector3f.add(new Vector3f(X * scale.x, Y * scale.y, Z * scale.z), translate);
                    vertices.add(v);
                } else if (line.startsWith("vt ")) {
                    String data = line.replace("vt ", "");
                    int separator1 = data.indexOf(" ");
                    String a = data.substring(0, separator1).trim();
                    String b = data.substring(separator1).trim();
                    float x = Float.parseFloat(a);
                    float y = Float.parseFloat(b);
                    Vector2f v = new Vector2f(x, y);
                    colorPos.add(v);
                } else if (line.startsWith("f ")) {
                    String data = line.replace("f ", "");
                    int separator1 = data.indexOf(" ");
                    String a = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String b = data.substring(separator1, separator2).trim();
                    String c = data.substring(separator2).trim();
                    // Indexes
                    int vIndexA = a.indexOf("/");
                    int vIndexB = b.indexOf("/");
                    int vIndexC = c.indexOf("/");
                    String aVIdx = a.substring(0, vIndexA);
                    String bVIdx = b.substring(0, vIndexB);
                    String cVIdx = c.substring(0, vIndexC);
                    int vAi = Integer.parseInt(aVIdx) - 1;
                    int vBi = Integer.parseInt(bVIdx) - 1;
                    int vCi = Integer.parseInt(cVIdx) - 1;
                    if (line.contains("//")) {
                        Vector3f tCol = new Vector3f(1.0f);
                        if (image != null && !colorPos.isEmpty()) {
                            String A = a.substring(vIndexA + 2);
                            String B = a.substring(vIndexB + 2);
                            String C = a.substring(vIndexC + 2);
                            int aa = Integer.parseInt(A);
                            int bb = Integer.parseInt(B);
                            int cc = Integer.parseInt(C);
                            Vector2f posA = colorPos.get(aa);
                            Vector2f posB = colorPos.get(bb);
                            Vector2f posC = colorPos.get(cc);
                            Vector3f cA = toColor(image.getRGB((int) (posA.x * (width - 1.0f)), (int) ((1.0f - posA.y) * (height - 1.0f))));
                            Vector3f cB = toColor(image.getRGB((int) (posB.x * (width - 1.0f)), (int) ((1.0f - posB.y) * (height - 1.0f))));
                            Vector3f cC = toColor(image.getRGB((int) (posC.x * (width - 1.0f)), (int) ((1.0f - posC.y) * (height - 1.0f))));
                            tCol = Vector3f.mul(Vector3f.add(Vector3f.add(cA, cB), cC), 1.0f / 3.0f);
                        } else if (currCol != null) {
                            tCol = currCol;
                        }
                        Triangle t = new Triangle(vertices.get(vAi), vertices.get(vBi), vertices.get(vCi), tCol);
                        triangles.add(t);
                        objectTriangles.add(t);
                        continue;
                    }
                    int colA = a.indexOf("/", vIndexA + 1);
                    int colB = b.indexOf("/", vIndexB + 1);
                    int colC = c.indexOf("/", vIndexC + 1);
                    // Parsing
                    String icolA = a.substring(vIndexA + 1, colA);
                    String icolB = b.substring(vIndexB + 1, colB);
                    String icolC = c.substring(vIndexC + 1, colC);
                    int aColI = Integer.parseInt(icolA) - 1;
                    int bColI = Integer.parseInt(icolB) - 1;
                    int cColI = Integer.parseInt(icolC) - 1;
                    Vector2f posA = colorPos.get(aColI);
                    Vector2f posB = colorPos.get(bColI);
                    Vector2f posC = colorPos.get(cColI);
                    Vector3f triangleCol = new Vector3f(1.0f);
                    if (image != null) {
                        Vector3f cA = toColor(image.getRGB((int) (posA.x * (width - 1.0f)), (int) ((1.0f - posA.y) * (height - 1.0f))));
                        Vector3f cB = toColor(image.getRGB((int) (posB.x * (width - 1.0f)), (int) ((1.0f - posB.y) * (height - 1.0f))));
                        Vector3f cC = toColor(image.getRGB((int) (posC.x * (width - 1.0f)), (int) ((1.0f - posC.y) * (height - 1.0f))));
                        triangleCol = Vector3f.mul(Vector3f.add(Vector3f.add(cA, cB), cC), 1.0f / 3.0f);
                    } else if (currCol != null) {
                        triangleCol = currCol;
                    }
                    Triangle t = new Triangle(vertices.get(vAi), vertices.get(vBi), vertices.get(vCi), triangleCol);
                    triangles.add(t);
                    objectTriangles.add(t);
                } else if (line.startsWith("vn ")) {
                    String data = line.replace("vn ", "");
                    int separator1 = data.indexOf(" ");
                    String x = data.substring(0, separator1).trim();
                    int separator2 = data.indexOf(" ", separator1 + 1);
                    String y = data.substring(separator1, separator2).trim();
                    String z = data.substring(separator2).trim();
                    float X = Float.parseFloat(x);
                    float Y = Float.parseFloat(y);
                    float Z = Float.parseFloat(z);
                    Vector3f v = new Vector3f(X, Y, Z);
                    normals.add(v);
                } else if (line.startsWith("usemtl ")) {
                    String data = line.replace("usemtl ", "").trim().toLowerCase();
                    if (!objectTriangles.isEmpty()) {
                        objects.add(objectTriangles);
                        objectNames.add(data);
                        objectTriangles.clear();
                    }
                    boolean doNull = true;
                    for (String filepath : filepaths) {
                        String objName = filepath.substring(0, filepath.indexOf(" ")).trim();
                        String fileName = filepath.substring(filepath.indexOf(" ") + 1).trim();
                        if (data.equals(objName)) {
                            File f = new File(texturePath + File.separator + fileName);
                            if (f.exists()) {
                                image = ImageIO.read(f);
                                width = image.getWidth();
                                height = image.getHeight();
                            }
                            doNull = false;
                            break;
                        }
                    }
                    if (doNull) {
                        image = null;
                    }
                    doNull = true;
                    for (int i = 0; i < cols.size(); i++) {
                        String n = materialColorNames.get(i);

                        if (data.equals(n)) {
                            currCol = cols.get(i);
                            doNull = false;
                            break;
                        }
                    }
                    if (doNull) {
                        currCol = null;
                    }
                }
            }
        } catch (IOException e) {
        }
        colorPos.clear();
    }

    private String findTexturesDirectory(String startingPath) {
        File startDirectory = new File(startingPath);
        if (!startDirectory.isDirectory()) {
            return null;
        }
        return findTexturesDirectoryRecursive(startDirectory);
    }

    private String findTexturesDirectoryRecursive(File directory) {
        File[] files = directory.listFiles();
        for (File file : files) {
            if (file.isDirectory() && file.getName().equalsIgnoreCase("textures")) {
                return file.getAbsolutePath();
            } else if (file.isDirectory()) {
                String result = findTexturesDirectoryRecursive(file);
                if (result != null) {
                    return result;
                }
            }
        }
        return null;
    }

    /**
     * Centers the loaded triangles, translating them won't make a difference.
     * centers by the bounding box, not by the center of mass.
     */
    public void center() {
        Vector3f ver = vertices.get(0);
        float minX = ver.x, maxX = ver.x, minY = ver.y, maxY = ver.y, minZ = ver.z, maxZ = ver.z;
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
        vertices.clear();
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

    public ArrayList<Triangle> getObject(int index) {
        if (index < 0 || index >= objects.size()) {
            String err = "There are only " + objects.size() + " objects!";
            throw new ArrayIndexOutOfBoundsException(err);
        }
        return objects.get(index);
    }

    /**
     * @return All object names
     */
    public ArrayList<String> getObjectNames() {
        return objectNames;
    }

    /**
     * @return The amount of objects
     */
    public int getObjectAmount() {
        return objects.size();
    }

    /**
     * Returns what file format it accepts, could be useful when dealingwith
     * both OBJ and STL loaders.
     *
     * @return The File extension
     */
    public String getExtension() {
        return "obj";
    }

    private static Vector3f toColor(int rgb) {
        return new Vector3f(((rgb >> 16) & 0xFF) / 255.0f, ((rgb >> 8) & 0xFF) / 255.0f, (rgb & 0xFF) / 255.0f);
    }
}
