package polyraygameengine.renderer;

import java.awt.geom.AffineTransform;
import java.util.ArrayList;
import static org.lwjgl.glfw.GLFW.*;
import static org.lwjgl.opengl.GL11.*;

public class Renderer {

    private ArrayList<Renderable> objects = new ArrayList<>();

    private AffineTransform cameraTransform;

    private GLFramebuffer frameBuffer;
    private GLTexture rendered, output;
    private PostProcess post;
    private FullscreenQuad quad;

    public Renderer(AffineTransform cameraTransform, int width, int height) {
        this.cameraTransform = cameraTransform;
        this.frameBuffer = new GLFramebuffer(width, height);
        this.rendered = frameBuffer.render;
        this.output = new GLTexture(width, height, GL_RGBA8, GL_RGBA, true, false);
        this.post = new PostProcess(rendered, output, 0, 1);
        
        this.quad = new FullscreenQuad();
    }

    public void changeEffect(int effect) {
        post.changeEffect(effect);
    }

    public void addObject(Renderable r) {
        objects.add(r);
    }

    public void clearObjects() {
        objects.clear();
    }

    public void render(GLFWindow w) {
        long window = w.getWindow();
        frameBuffer.bind();
        glViewport(0, 0, w.getWidth(), w.getHeight());
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (Renderable object : objects) {
            object.render(w.getWidth(), w.getHeight(), cameraTransform);
        }

        frameBuffer.unbind();
        
        post.runPostProcess();
        
        glClear(GL_COLOR_BUFFER_BIT);
        quad.setTexture(output);
        quad.render();

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}