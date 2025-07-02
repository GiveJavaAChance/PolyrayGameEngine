package polyray.builtin;

import polyray.modular.RendererBase;
import java.util.ArrayList;
import polyray.Background;
import polyray.BindingRegistry;
import static org.lwjgl.opengl.GL43.*;
import polyray.modular.Renderable;

public class Renderer3D extends RendererBase {

    private final ArrayList<Renderable> objects2D;
    private final ArrayList<Renderable> objects3D;

    private boolean showWireframe = false;

    private Background background;

    public Renderer3D(int width, int height) {
        super(width, height);
        this.objects2D = new ArrayList<>();
        this.objects3D = new ArrayList<>();
    }

    public Renderer3D(int width, int height, int MSAASampleCount) {
        super(width, height, MSAASampleCount);
        this.objects2D = new ArrayList<>();
        this.objects3D = new ArrayList<>();
    }

    public void setBackground(Background b) {
        this.background = b;
    }

    public void toggleWireframe() {
        this.showWireframe = !this.showWireframe;
    }

    public void add2DObject(Renderable r) {
        objects2D.add(r);
    }

    public void add3DObject(Renderable r) {
        objects3D.add(r);
    }

    public void remove2DObject(Renderable r) {
        objects2D.remove(r);
    }

    public void remove3DObject(Renderable r) {
        objects3D.remove(r);
    }

    public void clearObjects() {
        objects2D.clear();
        objects3D.clear();
    }

    @Override
    protected void renderInternal() {
        if (background != null) {
            background.render();
        }

        glEnable(GL_DEPTH_TEST);

        if (showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        for (Renderable object : objects3D) {
            object.render();
        }

        if (showWireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glDisable(GL_DEPTH_TEST);

        for (Renderable object : objects2D) {
            object.render();
        }
    }
}
