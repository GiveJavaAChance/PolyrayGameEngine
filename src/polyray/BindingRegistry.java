package polyray;

import java.util.HashMap;
import static org.lwjgl.opengl.GL43.*;

public class BindingRegistry {

    private static final HashMap<Integer, Integer> usedImageBindings = new HashMap<>();
    private static final HashMap<Integer, Integer> usedBufferBindings = new HashMap<>();

    public static int bindImageTexture(GLTexture texture, int level, boolean layered, int layer, int access) {
        return bindImageTexture(texture, level, layered, layer, access, texture.format);
    }

    public static int bindImageTexture(GLTexture texture, int level, boolean layered, int layer, int access, int format) {
        Integer currentBinding = usedImageBindings.get(texture.ID);
        if (currentBinding != null) {
            return currentBinding;
        }
        int binding = getNewBinding(usedImageBindings, glGetInteger(GL_MAX_IMAGE_UNITS));
        if (binding == -1) {
            System.err.println("No available image binding points!");
            return -1;
        }
        glBindImageTexture(binding, texture.ID, level, layered, layer, access, format);
        usedImageBindings.put(texture.ID, binding);
        return binding;
    }

    public static int bindBufferBase(ShaderBuffer buffer) {
        return bindBufferBase(buffer, buffer.target);
    }

    public static int bindBufferBase(ShaderBuffer buffer, int target) {
        Integer currentBinding = usedBufferBindings.get(buffer.ID);
        if (currentBinding != null) {
            return currentBinding;
        }
        int binding = getNewBinding(usedBufferBindings, glGetInteger(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS));
        if (binding == -1) {
            System.err.println("No available image binding points!");
            return -1;
        }
        glBindBufferBase(target, binding, buffer.ID);
        usedBufferBindings.put(buffer.ID, binding);
        return binding;
    }

    private static int getNewBinding(HashMap<Integer, Integer> usedBindings, int range) {
        int maxSearchRange = 0;
        for (int i : usedBindings.values()) {
            if (i > maxSearchRange) {
                maxSearchRange = i;
            }
        }
        maxSearchRange += 2;
        if (maxSearchRange > range) {
            maxSearchRange = range;
        }
        for (int i = 0; i < maxSearchRange; i++) {
            if (!usedBindings.containsValue(i)) {
                return i;
            }
        }
        return -1;
    }

    public static void unbindImageTexture(GLTexture texture) {
        usedImageBindings.remove(texture.ID);
    }

    public static void unbindImageTexture(ShaderBuffer buffer) {
        usedBufferBindings.remove(buffer.ID);
    }
}
