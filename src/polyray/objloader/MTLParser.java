package polyray.objloader;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.util.HashMap;
import polyray.Vector3f;

public class MTLParser {

    public static final HashMap<String, OBJMaterial> loadMTLFile(File file) throws IOException {
        if (!file.exists()) {
            return null;
        }
        String parentStr = file.getParentFile().getAbsolutePath() + "\\";
        HashMap<String, OBJMaterial> materials = new HashMap<>();

        BufferedReader reader = new BufferedReader(new FileReader(file));
        OBJMaterial currentOBJMaterial = null;
        String line;
        while ((line = reader.readLine()) != null) {
            line = line.trim();
            if (line.startsWith("#") || line.isEmpty()) {
                continue;
            }
            String[] tokens = line.split("\\s+");
            String f = parentStr + tokens[1];
            if (tokens[0].matches("map_Kd|map_Ns|map_refl|map_Bump") && f.endsWith(".dds")) {
                throw new IOException("Unsupported Image Texture Format: " + f);
            }
            if (tokens[0].equals("newmtl")) {
                currentOBJMaterial = new OBJMaterial(tokens[1]);
                materials.put(currentOBJMaterial.name, currentOBJMaterial);
                continue;
            }
            if (currentOBJMaterial == null) {
                continue;
            }
            switch (tokens[0]) {
                case "Kd" -> {
                    currentOBJMaterial.Kd = parseColor(tokens);
                }
                case "Ks" -> {
                    currentOBJMaterial.Ks = parseColor(tokens);
                }
                case "Ke" -> {
                    currentOBJMaterial.Ke = parseColor(tokens);
                }
                case "Ka" -> {
                    currentOBJMaterial.Ka = parseColor(tokens);
                }
                case "Ns" -> {
                    currentOBJMaterial.Ns = Float.parseFloat(tokens[1]);
                }
                case "Ni" -> {
                    currentOBJMaterial.Ni = Float.parseFloat(tokens[1]);
                }
                case "d" -> {
                    currentOBJMaterial.d = Float.parseFloat(tokens[1]);
                }
                case "illum" -> {
                    currentOBJMaterial.illum = Integer.parseInt(tokens[1]);
                }
                case "map_Kd" -> {
                    currentOBJMaterial.mapKd = f;
                }
                case "map_Ns" -> {
                    currentOBJMaterial.mapNs = f;
                }
                case "map_refl" -> {
                    currentOBJMaterial.mapRefl = f;
                }
                case "map_Bump" -> {
                    currentOBJMaterial.mapBump = f;
                }
            }
        }
        reader.close();
        return materials;
    }

    private static Vector3f parseColor(String[] tokens) {
        return new Vector3f(
                Float.parseFloat(tokens[1]),
                Float.parseFloat(tokens[2]),
                Float.parseFloat(tokens[3])
        );
    }
}
