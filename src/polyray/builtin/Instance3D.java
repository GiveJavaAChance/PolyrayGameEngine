package polyray.builtin;

import polyray.modular.Instance;
import java.nio.FloatBuffer;
import polyray.Transform3D;
import polyray.VertexBufferTemplate;
import static polyray.VertexBufferTemplate.VertexAttribute.*;

public class Instance3D implements Instance {

    public static final VertexBufferTemplate VBO_TEMPLATE = new VertexBufferTemplate(true, VEC4, VEC4, VEC4, VEC4);

    public Transform3D transform;

    public Instance3D(Transform3D tx) {
        this.transform = tx;
    }

    @Override
    public void toFloatBuffer(FloatBuffer buffer) {
        transform.toFloatBuffer(buffer);
    }

}
