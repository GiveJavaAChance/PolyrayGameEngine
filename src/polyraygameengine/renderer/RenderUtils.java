package polyraygameengine.renderer;

import java.awt.image.BufferedImage;

public class RenderUtils {

    public static final Renderable createRenderableFromImage(BufferedImage image) {
        int w = image.getWidth();
        int h = image.getHeight();
        Renderable r = new Renderable(new Texture(image), ShaderProgram.fromFiles("quad.vert", "quad.frag", "quad", 0));
        r.addTriangle(new Vertex(0.0f, 0.0f, 0.0f, 0.0f), new Vertex(w, 0.0f, 1.0f, 0.0f), new Vertex(w, -h, 1.0f, 1.0f));
        r.addTriangle(new Vertex(0.0f, 0.0f, 0.0f, 0.0f), new Vertex(0.0f, -h, 0.0f, 1.0f), new Vertex(w, -h, 1.0f, 1.0f));
        r.upload();
        return r;
    }
}
