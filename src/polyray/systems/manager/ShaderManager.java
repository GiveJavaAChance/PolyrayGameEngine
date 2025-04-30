package polyray.systems.manager;

import java.util.ArrayList;
import polyray.Material;
import polyray.ShaderPreprocessor;
import polyray.ShaderProgram;
import polyray.Vector3f;

public class ShaderManager {

    private static final ArrayList<ShaderValue> values = new ArrayList<>();

    public static final void addValue(String name, Object value) {
        values.add(new ShaderValue(name, value));
    }

    public static final ShaderProgram createProgram(String vert, String frag, String name, int index) {
        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles(vert, frag);
        proc.appendAll();
        setValues(proc);
        return proc.createProgram(name, index);
    }

    public static final ShaderPreprocessor createProcessor(String vert, String frag) {
        ShaderPreprocessor proc = ShaderPreprocessor.fromFiles(vert, frag);
        proc.appendAll();
        setValues(proc);
        return proc;
    }

    public static final Material createDefaultMaterial(String vert, String frag, String name, int index) {
        Material mat = new Material(createProgram(vert, frag, name, index));
        mat.setRoughness(0.5f);
        mat.setMetallic(0.5f);
        mat.setF0(new Vector3f(0.05f, 0.05f, 0.05f));
        return mat;
    }

    private static void setValues(ShaderPreprocessor proc) {
        for (ShaderValue v : values) {
            switch (v.value) {
                case Integer i -> {
                    proc.setInt(v.name, i);
                }
                case Float f -> {
                    proc.setFloat(v.name, f);
                }
                case Double d -> {
                    proc.setDouble(v.name, d);
                }
                case String s -> {
                    proc.setString(v.name, s);
                }
                default -> {

                }
            }
        }
    }

    private static class ShaderValue {

        public final String name;
        public final Object value;

        public ShaderValue(String name, Object value) {
            this.name = name;
            this.value = value;
        }
    }
}
