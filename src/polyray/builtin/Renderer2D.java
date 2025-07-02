package polyray.builtin;

import java.util.ArrayList;
import polyray.modular.Renderable;
import polyray.modular.RendererBase;

public class Renderer2D extends RendererBase {

    private final ArrayList<Renderable> objects;

    public Renderer2D(int width, int height) {
        super(width, height);
        this.objects = new ArrayList<>();
    }

    public Renderer2D(int width, int height, int MSAASampleCount) {
        super(width, height, MSAASampleCount);
        this.objects = new ArrayList<>();
    }

    public void addObject(Renderable obj) {
        this.objects.add(obj);
    }

    public void removeObject(Renderable obj) {
        this.objects.remove(obj);
    }

    public void clearObjects() {
        this.objects.clear();
    }

    @Override
    protected void renderInternal() {
        for (Renderable object : objects) {
            object.render();
        }
    }
}
