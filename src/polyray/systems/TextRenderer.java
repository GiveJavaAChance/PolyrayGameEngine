package polyray.systems;

import polyray.systems.manager.*;
import java.util.HashMap;
import java.util.HashSet;
import static org.lwjgl.opengl.GL43.*;
import polyray.BindingRegistry;
import polyray.GLTexture.GLTextureArray;
import polyray.ShaderBuffer;
import polyray.ShaderBufferHeap;
import polyray.ShaderPreprocessor;
import polyray.ShaderProgram;
import polyray.Vector2f;
import polyray.builtin.Vertex2D;
import polyray.modular.Renderable;

public class TextRenderer implements Renderable {

    private static final int INTS_PER_STRING = 2;

    private final HashMap<Integer, Integer> lengths;
    private final ShaderBufferHeap stringBuffer;
    private final char firstChar;
    private final int stringBufferBinding;
    private final GLTextureArray bitmap;
    private final int vao;
    private final ShaderBuffer vbo;
    private final ShaderBuffer drawBuffer;
    private final ShaderProgram shader;
    public final int characterWidth;
    public final int characterHeight;

    private boolean update = true;

    private TextRenderer(int bytes, GLTextureArray bitmap, char startingChar, float[] verts, int characterWidth, int characterHeight) {
        this.lengths = new HashMap<>();
        this.stringBuffer = new ShaderBufferHeap(bytes >> 2, 1);
        this.stringBufferBinding = BindingRegistry.bindBufferBase(stringBuffer.buffer);
        ShaderPreprocessor proc = ShaderManager.createProcessor("Text.vert", "Text.frag");
        proc.setInt("STR_IDX", stringBufferBinding);
        proc.setInt("INTS_PER_STRING", INTS_PER_STRING);
        proc.setInt("CHARACTER_WIDTH", characterWidth);
        this.shader = proc.createProgram();
        this.firstChar = startingChar;
        this.drawBuffer = new ShaderBuffer(GL_DRAW_INDIRECT_BUFFER, GL_DYNAMIC_DRAW);
        this.bitmap = bitmap;
        this.vao = glGenVertexArrays();
        glBindVertexArray(vao);
        this.vbo = new ShaderBuffer(GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW, Vertex2D.VBO_TEMPLATE.build(0));
        glBindVertexArray(0);
        vbo.uploadData(verts);
        this.characterWidth = characterWidth;
        this.characterHeight = characterHeight;
    }

    public TextRenderer(int bytes, GLTextureArray bitmap, char startingChar, Vector2f rectMin, Vector2f rectMax) {
        this(bytes, bitmap, startingChar, new float[]{
            rectMin.x, rectMin.y, 0.0f, 0.0f,
            rectMax.x, rectMin.y, 1.0f, 0.0f,
            rectMin.x, rectMax.y, 0.0f, 1.0f,
            rectMax.x, rectMax.y, 1.0f, 1.0f
        }, (int) Math.abs(rectMax.x - rectMin.x), (int) Math.abs(rectMax.y - rectMin.y));
    }

    public TextRenderer(int bytes, GLTextureArray bitmap, char startingChar, boolean flipY) {
        this(bytes, bitmap, startingChar, new Vector2f(0.0f, flipY ? 0.0f : bitmap.getHeight()), new Vector2f(bitmap.getWidth(), flipY ? bitmap.getHeight() : 0.0f));
    }

    public void update() {
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

    public int push(String str, int color, int x, int y) {
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

    public int pushCentered(String str, int color, int x, int y) {
        return push(str, color, x - ((str.length() * characterWidth) >> 1), y - (characterHeight >> 1));
    }

    public void pop(int ID) {
        stringBuffer.free(ID);
        lengths.remove(ID);
        update = true;
    }

    public void setVisible(int ID, boolean visible) {
        if (visible) {
            stringBuffer.activate(ID);
        } else {
            stringBuffer.deactivate(ID);
        }
        update = true;
    }

    public void modify(int ID, int newColor, int newX, int newY) {
        int packedPos = ((newX + 32768) & 0xFFFF) << 16 | ((newY + 32768) & 0xFFFF);
        int[] data = new int[INTS_PER_STRING];
        data[0] = packedPos;
        data[1] = newColor;
        stringBuffer.write(ID, 0, data);
        update = true;
    }

    private int getChar(char[] c, int idx) {
        if (idx >= c.length) {
            return 0;
        }
        return c[idx] - firstChar;
    }

    @Override
    public void render() {
        shader.use();
        glActiveTexture(GL_TEXTURE0);
        bitmap.bind();
        glBindVertexArray(vao);
        drawBuffer.bind();
        glMultiDrawArraysIndirect(GL_TRIANGLE_STRIP, 0, stringBuffer.getActive().size(), 16);
        shader.unuse();
    }
}
