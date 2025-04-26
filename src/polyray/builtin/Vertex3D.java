package polyray.builtin;

import polyray.modular.Vertex;
import java.nio.FloatBuffer;
import polyray.VertexBufferTemplate;
import polyray.VertexBufferTemplate.VertexAttribute;

public class Vertex3D implements Vertex {
    
    public static final VertexBufferTemplate VBO_TEMPLATE = new VertexBufferTemplate(false)
            .addAttribute(VertexAttribute.VEC3)
            .addAttribute(VertexAttribute.VEC3)
            .addAttribute(VertexAttribute.VEC2);

    public float x, y, z, nx, ny, nz, u, v;

    public Vertex3D(float x, float y, float z, float nx, float ny, float nz, float u, float v) {
        this.x = x;
        this.y = y;
        this.z = z;
        this.nx = nx;
        this.ny = ny;
        this.nz = nz;
        this.u = u;
        this.v = v;
    }

    @Override
    public void put(FloatBuffer buffer) {
        buffer.put(x).put(y).put(z).put(nx).put(ny).put(nz).put(u).put(v);
    }
}
