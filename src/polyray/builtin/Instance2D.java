package polyray.builtin;

import polyray.modular.Instance;
import java.nio.FloatBuffer;
import polyray.Transform2D;
import polyray.VertexBufferTemplate;
import static polyray.VertexBufferTemplate.VertexAttribute.*;

public class Instance2D implements Instance {

    public static final VertexBufferTemplate VBO_TEMPLATE = new VertexBufferTemplate(true, VEC3, VEC3, VEC3);

    public Transform2D transform;

    public Instance2D(Transform2D transform) {
        this.transform = transform;
    }

    @Override
    public void toFloatBuffer(FloatBuffer buffer) {
        this.transform.toFloatBuffer(buffer, false);
    }
}
