package polyray;

import java.util.ArrayList;
import static org.lwjgl.opengl.GL43.*;

public class VertexBufferTemplate {

    private final ArrayList<VertexAttribute> attributes = new ArrayList<>();
    private int totalSize = 0;
    private final boolean instance;

    public VertexBufferTemplate(boolean instance) {
        this.instance = instance;
    }

    public VertexBufferTemplate addAttribute(VertexAttribute attribute) {
        this.attributes.add(attribute);
        this.totalSize += attribute.size;
        return this;
    }

    public int build(int idx) {
        int vbo = glGenBuffers();
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        int off = 0;
        for (VertexAttribute attribute : attributes) {
            attribute.bind(idx, off, totalSize);
            if (instance) {
                glVertexAttribDivisor(idx, 1);
            }
            idx++;
            off += attribute.size;
        }
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return vbo;
    }

    public int getSize() {
        return this.totalSize;
    }
    
    public int getAttributeCount() {
        return this.attributes.size();
    }

    public static class VertexAttribute {

        public static final VertexAttribute FLOAT = new VertexAttribute(1, GL_FLOAT, false);
        public static final VertexAttribute VEC2 = new VertexAttribute(2, GL_FLOAT, false);
        public static final VertexAttribute VEC3 = new VertexAttribute(3, GL_FLOAT, false);
        public static final VertexAttribute VEC4 = new VertexAttribute(4, GL_FLOAT, false);
        public static final VertexAttribute INT = new VertexAttribute(1, GL_INT, false);
        public static final VertexAttribute IVEC2 = new VertexAttribute(2, GL_INT, false);
        public static final VertexAttribute IVEC3 = new VertexAttribute(3, GL_INT, false);
        public static final VertexAttribute IVEC4 = new VertexAttribute(4, GL_INT, false);

        public final int size;
        private final int type;
        private final boolean normalized;

        public VertexAttribute(int size, int type, boolean normalized) {
            this.size = size;
            this.type = type;
            this.normalized = normalized;
        }

        protected void bind(int idx, int off, int totalSize) {
            glVertexAttribPointer(idx, this.size, this.type, this.normalized, totalSize * Float.BYTES, off * Float.BYTES);
            glEnableVertexAttribArray(idx);
        }
    }
}
