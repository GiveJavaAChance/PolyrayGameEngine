package polyray.systems.manager;

import java.util.HashMap;
import java.util.HashSet;
import static org.lwjgl.opengl.GL43.*;
import polyray.BindingRegistry;
import polyray.GLTexture.GLTextureArray;
import polyray.ShaderBuffer;
import polyray.ShaderBufferHeap;
import polyray.ShaderPreprocessor;
import polyray.ShaderProgram;
import polyray.builtin.Vertex2D;
import polyray.modular.Renderable;

public class TextRenderManager {

    private static final int INTS_PER_STRING = 2;

    private static final HashMap<Integer, Integer> lengths = new HashMap<>();
    private static ShaderBufferHeap stringBuffer;
    private static int stringIdx = -1;
    private static char firstChar;

    private static GLTextureArray texture;
    private static int vao;
    private static ShaderBuffer vbo;
    private static ShaderBuffer drawBuffer;
    private static ShaderProgram shader;

    private static boolean update = true;

    public static Renderable setup(int bytes, GLTextureArray characterBitmaps, char startingChar, boolean flipY) {
        stringBuffer = new ShaderBufferHeap(bytes >> 2, 1);
        stringIdx = BindingRegistry.bindBufferBase(stringBuffer.buffer);
        firstChar = startingChar;
        drawBuffer = new ShaderBuffer(GL_DRAW_INDIRECT_BUFFER, GL_DYNAMIC_DRAW);
        texture = characterBitmaps;
        vao = glGenVertexArrays();
        glBindVertexArray(vao);
        vbo = new ShaderBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, Vertex2D.VBO_TEMPLATE.build(0));
        glBindVertexArray(0);
        float v = flipY ? 0.0f : 1.0f;
        float[] data = {
            0.0f, 0.0f, 0.0f, v,
            characterBitmaps.getWidth(), 0.0f, 1.0f, v,
            0.0f, characterBitmaps.getHeight(), 0.0f, 1.0f - v,
            characterBitmaps.getWidth(), characterBitmaps.getHeight(), 1.0f, 1.0f - v
        };
        vbo.uploadData(data);
        ShaderPreprocessor proc = ShaderManager.createProcessor("Text.vert", "Text.frag");
        proc.setInt("STR_IDX", stringIdx);
        proc.setInt("INTS_PER_STRING", INTS_PER_STRING);
        shader = proc.createProgram();
        return TextRenderManager::render;
    }

    public static void update() {
        if (!update) {
            return;
        }
        update = false;
        HashSet<Integer> active = stringBuffer.getActive();
        HashMap<Integer, Long> strings = stringBuffer.getAllocations();
        int[] drawData = new int[active.size() << 2];
        int idx = 0;
        for (int id : active) {
            long m = strings.get(id);
            int pointer = (int) (m >> 32l) & 0xFFFFFFFF;
            int length = lengths.get(id);
            drawData[idx++] = 4;
            drawData[idx++] = length;
            drawData[idx++] = 0;
            drawData[idx++] = pointer;
        }
        drawBuffer.uploadData(drawData);
        stringBuffer.pollUpdates();
    }

    public static int push(String str, int color, int x, int y) {
        char[] chars = str.toCharArray();
        int length = (chars.length + 3) / 4 + INTS_PER_STRING;
        int packedPos = ((x + 32768) & 0xFFFF) << 16 | ((y + 32768) & 0xFFFF);
        int[] data = new int[length];
        data[0] = packedPos;
        data[1] = color;
        for (int i = 0; i < length - INTS_PER_STRING; i++) {
            int idx = i << 2;
            int packedChars = getChar(chars, idx) << 24 | getChar(chars, idx + 1) << 16 | getChar(chars, idx + 2) << 8 | getChar(chars, idx + 3);
            data[i + INTS_PER_STRING] = packedChars;
        }
        int id = stringBuffer.allocate(data);
        lengths.put(id, chars.length);
        update = true;
        return id;
    }

    public static int pushCentered(String str, int color, int x, int y) {
        return push(str, color, x - ((str.length() * 11) >> 1), y - 7);
    }

    public static void pop(int ID) {
        stringBuffer.free(ID);
        lengths.remove(ID);
        update = true;
    }

    public static void setVisible(int ID, boolean visible) {
        if (visible) {
            stringBuffer.activate(ID);
        } else {
            stringBuffer.deactivate(ID);
        }
        update = true;
    }
    
    public static void modify(int ID, int newColor, int newX, int newY) {
        int packedPos = ((newX + 32768) & 0xFFFF) << 16 | ((newY + 32768) & 0xFFFF);
        int[] data = new int[INTS_PER_STRING];
        data[0] = packedPos;
        data[1] = newColor;
        stringBuffer.write(ID, 0, data);
        update = true;
    }

    private static int getChar(char[] c, int idx) {
        if (idx >= c.length) {
            return 0;
        }
        return c[idx] - firstChar;
    }

    private static void render() {
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        texture.bind();
        glBindVertexArray(vao);
        drawBuffer.bind();
        glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, stringBuffer.getActive().size(), 16);
        shader.unuse();
    }
}
