package polyray.systems.manager;

import java.util.HashMap;
import java.util.function.Consumer;
import polyray.Material;
import polyray.ShaderPreprocessor;
import polyray.ShaderProgram;
import polyray.Vector3f;

public class ShaderManager {

    private static final HashMap<String, Consumer<ShaderPreprocessor>> values = new HashMap<>();

    public static final void addValue(String name, int value) {
        values.put(name, proc -> proc.setInt(name, value));
    }

    public static final void addValue(String name, float value) {
        values.put(name, proc -> proc.setFloat(name, value));
    }

    public static final void addValue(String name, double value) {
        values.put(name, proc -> proc.setDouble(name, value));
    }

    public static final void addValue(String name, String value) {
        values.put(name, proc -> proc.setString(name, value));
    }

    public static final ShaderProgram createProgram(String vert, String frag) {
        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles(vert, frag);
        proc.appendAll();
        setValues(proc);
        return proc.createProgram();
    }

    public static final ShaderPreprocessor createProcessor(String vert, String frag) {
        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles(vert, frag);
        proc.appendAll();
        setValues(proc);
        return proc;
    }

    public static final Material createDefaultMaterial(String vert, String frag) {
        Material mat = new Material(createProgram(vert, frag));
        mat.setRoughness(0.5f);
        mat.setMetallic(0.5f);
        mat.setF0(new Vector3f(0.05f, 0.05f, 0.05f));
        return mat;
    }

    private static void setValues(ShaderPreprocessor proc) {
        for (Consumer<ShaderPreprocessor> v : values.values()) {
            v.accept(proc);
        }
    }
}
