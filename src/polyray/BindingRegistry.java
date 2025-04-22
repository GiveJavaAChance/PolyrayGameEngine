package polyray;

import java.util.HashMap;
import static org.lwjgl.opengl.GL43.*;

public class BindingRegistry {

    private static final HashMap<String, Integer> usedBindings = new HashMap<>();

    public static int bindImageTexture(GLTexture texture, int level, boolean layered, int layer, int access) {
        return bindImageTexture(texture, level, layered, layer, access, texture.interFormat);
    }

    public static int bindImageTexture(GLTexture texture, int level, boolean layered, int layer, int access, int format) {
        String key = "tex" + texture.ID;
        Integer currentBinding = usedBindings.get(key);
        if (currentBinding != null) {
            return currentBinding;
        }
        int binding = getNextAvailableBinding();
        if (binding == -1) {
            System.err.println("No available image binding points!");
            return -1;
        }
        glBindImageTexture(binding, texture.ID, level, layered, layer, access, format);
        usedBindings.put(key, binding);
        return binding;
    }

    public static int bindBufferBase(ShaderBuffer buffer) {
        return bindBufferBase(buffer, buffer.target);
    }

    public static int bindBufferBase(ShaderBuffer buffer, int target) {
        String key = "buf" + buffer.ID;
        Integer currentBinding = usedBindings.get(key);
        if (currentBinding != null) {
            return currentBinding;
        }
        int binding = getNextAvailableBinding();
        if (binding == -1) {
            System.err.println("No available image binding points!");
            return -1;
        }
        glBindBufferBase(target, binding, buffer.ID);
        usedBindings.put(key, binding);
        return binding;
    }

    private static int getNextAvailableBinding() {
        int maxSearchRange = 0;
        for (int i : usedBindings.values()) {
            if (i > maxSearchRange) {
                maxSearchRange = i;
            }
        }
        maxSearchRange += 2;
        if (maxSearchRange > GL_MAX_IMAGE_UNITS) {
            maxSearchRange = GL_MAX_IMAGE_UNITS;
        }
        for (int i = 0; i < maxSearchRange; i++) {
            if (!usedBindings.containsValue(i)) {
                return i;
            }
        }
        return -1;
    }

    public static void unbindImageTexture(GLTexture texture) {
        String key = "tex" + texture.ID;
        Integer b = usedBindings.get(key);
        if (b != null) {
            glBindImageTexture(b, 0, 0, false, 0, GL_READ_WRITE, texture.interFormat);
            usedBindings.remove(key);
        }
    }

    public static void unbindImageTexture(ShaderBuffer buffer) {
        String key = "buf" + buffer.ID;
        Integer b = usedBindings.get(key);
        if (b != null) {
            glBindBufferBase(buffer.target, b, 0);
            usedBindings.remove(key);
        }
    }
}
