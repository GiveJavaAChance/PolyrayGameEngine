package polyray.systems.manager;

import java.util.BitSet;
import java.util.HashMap;
import static org.lwjgl.opengl.GL43.*;
import polyray.BindingRegistry;
import polyray.GLTexture.GLTextureArray;
import polyray.ShaderBuffer;
import polyray.ShaderPreprocessor;
import polyray.ShaderProgram;
import polyray.builtin.Vertex2D;
import polyray.modular.Renderable;
import polyray.systems.IDGenerator;

public class TextRenderManager {

    private static final int INTS_PER_STRING = 2;

    private static final IDGenerator gen = new IDGenerator();
    private static final HashMap<Integer, Long> strings = new HashMap<>();

    private static int VIRTUAL_RAM_SIZE;
    private static BitSet memoryMask;
    private static ShaderBuffer stringBuffer;

    private static int stringIdx = -1;
    private static ShaderBuffer drawBuffer;
    private static Text text;

    private static boolean update = true;

    public static final Renderable setup(int virtualRAM, GLTextureArray characterBitmaps, char staringChar) {
        VIRTUAL_RAM_SIZE = virtualRAM;
        memoryMask = new BitSet(VIRTUAL_RAM_SIZE >> 2);
        stringBuffer = new ShaderBuffer(GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
        stringIdx = BindingRegistry.bindBufferBase(stringBuffer);
        stringBuffer.setSize(VIRTUAL_RAM_SIZE);
        drawBuffer = new ShaderBuffer(GL_DRAW_INDIRECT_BUFFER, GL_DYNAMIC_DRAW);
        text = new Text(characterBitmaps);
        return text;
    }

    public static final void update() {
        if (!update) {
            return;
        }
        update = false;
        int[] drawData = new int[strings.size() << 2];
        int idx = 0;
        for (long v : strings.values()) {
            int pointer = (int) (v >> 32l) & 0xFFFFFFFF;
            int length = (int) (v & 0xFFFFFFFF);
            drawData[idx++] = 4;
            drawData[idx++] = length;
            drawData[idx++] = 0;
            drawData[idx++] = pointer;
        }
        drawBuffer.uploadData(drawData);
    }

    public static final int push(String str, int color, int x, int y) {
        char[] chars = str.toCharArray();
        int length = (chars.length + 3) / 4 + INTS_PER_STRING;
        int pointer = allocate(length);
        if (pointer == -1) {
            throw new OutOfMemoryError("Virtual String RAM full!");
        }
        int ID = gen.getNewID();
        strings.put(ID, (pointer & 0xFFFFFFFFl) << 32 | (chars.length & 0xFFFFFFFFl));
        int packedPos = ((x + 32768) & 0xFFFF) << 16 | ((y + 32768) & 0xFFFF);
        int[] data = new int[length];
        data[0] = packedPos;
        data[1] = color;
        for (int i = 0; i < length - INTS_PER_STRING; i++) {
            int idx = i << 2;
            int packedChars = getChar(chars, idx) << 24 | getChar(chars, idx + 1) << 16 | getChar(chars, idx + 2) << 8 | getChar(chars, idx + 3);
            data[i + INTS_PER_STRING] = packedChars;
        }
        stringBuffer.uploadPartialData(data, pointer);
        update = true;
        return ID;
    }

    public static final int pushCentered(String str, int color, int x, int y) {
        return push(str, color, x - ((str.length() * 11) >> 1), y - 7);
    }

    public static final void pop(int ID) {
        Long v = strings.remove(ID);
        if (v == null) {
            return;
        }
        int pointer = (int) (v >> 32l) & 0xFFFFFFFF;
        int length = (int) (v & 0xFFFFFFFF);
        memoryMask.clear(pointer, pointer + (length + 3) / 4 + INTS_PER_STRING);
        gen.freeID(ID);
        update = true;
    }

    private static int getChar(char[] c, int idx) {
        if (idx >= c.length) {
            return 0;
        }
        return c[idx] - ' ';
    }

    private static int allocate(int length) {
        int currentIdx = 0;
        while (currentIdx < memoryMask.size()) {
            int startIdx = memoryMask.nextClearBit(currentIdx);
            int endIdx = memoryMask.nextSetBit(startIdx);
            if (endIdx == -1) {
                endIdx = memoryMask.size();
            }
            if ((endIdx - startIdx) >= length) {
                memoryMask.set(startIdx, startIdx + length);
                return startIdx;
            }
            currentIdx = endIdx + 1;
        }
        return -1;
    }

    private static class Text implements Renderable {

        private final GLTextureArray texture;
        private final int vao;
        private final ShaderBuffer vbo;
        private final ShaderProgram shader;

        public Text(GLTextureArray tex) {
            this.texture = tex;
            this.vao = glGenVertexArrays();
            glBindVertexArray(vao);
            this.vbo = new ShaderBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, Vertex2D.VBO_TEMPLATE.build(0));
            glBindVertexArray(0);
            float[] data = {
                0.0f, 0.0f, 0.0f, 1.0f,
                tex.getWidth(), 0.0f, 1.0f, 1.0f,
                0.0f, tex.getHeight(), 0.0f, 0.0f,
                tex.getWidth(), tex.getHeight(), 1.0f, 0.0f
            };
            vbo.uploadData(data);
            ShaderPreprocessor proc = ShaderManager.createProcessor("Text.vert", "Text.frag");
            proc.setInt("STR_IDX", stringIdx);
            proc.setInt("INTS_PER_STRING", INTS_PER_STRING);
            this.shader = proc.createProgram();
        }

        @Override
        public void render() {
            shader.use();
            glActiveTexture(GL_TEXTURE0);
            texture.bind();
            glBindVertexArray(vao);
            drawBuffer.bind();
            glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, strings.size(), 16);
            shader.unuse();
        }

    }
}
