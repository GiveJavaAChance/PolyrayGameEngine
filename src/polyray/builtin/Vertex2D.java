package polyray.builtin;

import polyray.modular.Vertex;
import java.nio.FloatBuffer;
import polyray.VertexBufferTemplate;
import polyray.VertexBufferTemplate.VertexAttribute;

public class Vertex2D implements Vertex {

    public static final VertexBufferTemplate VBO_TEMPLATE = new VertexBufferTemplate(false)
            .addAttribute(VertexAttribute.VEC2)
            .addAttribute(VertexAttribute.VEC2);

    public float x, y, u, v;

    public Vertex2D(float x, float y, float u, float v) {
        this.x = x;
        this.y = y;
        this.u = u;
        this.v = v;
    }

    @Override
    public void put(FloatBuffer buffer) {
        buffer.put(x).put(y).put(u).put(v);
    }
}
