package polyraygameengine.renderer;

import java.awt.DisplayMode;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import static org.lwjgl.glfw.Callbacks.glfwFreeCallbacks;
import org.lwjgl.glfw.GLFW;
import static org.lwjgl.glfw.GLFW.*;
import org.lwjgl.glfw.GLFWErrorCallback;
import org.lwjgl.glfw.GLFWVidMode;
import org.lwjgl.opengl.GL;
import static org.lwjgl.opengl.GL11.*;

public class GLFWindow {

    private int width, height;
    private String name = "";
    private long window;
    private int exitKey = GLFW.GLFW_KEY_Q;

    public GLFWindow(String name) {
        this.name = name;
    }

    public void createFrame(int width, int height, boolean title, boolean fullscreen) {
        if (fullscreen) {
            GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
            GraphicsDevice gd = ge.getDefaultScreenDevice();
            DisplayMode dm = gd.getDisplayMode();
            this.width = dm.getWidth();
            this.height = dm.getHeight();
        } else {
            this.width = width;
            this.height = height;
        }

        if (!glfwInit()) {
            throw new IllegalStateException("Unable to initialize GLFW");
        }

        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        long monitor = glfwGetPrimaryMonitor(); // Get the primary monitor
        GLFWVidMode vidMode = glfwGetVideoMode(monitor); // Get the video mode of the primary monitor

        if (fullscreen) {
            // Create a full screen window
            if (title) {
                glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
            }
            window = glfwCreateWindow(vidMode.width(), vidMode.height(), name, monitor, 0l);
        } else {
            // Create a windowed mode window
            window = glfwCreateWindow(width, height, name, 0l, 0l);
        }

        if (window == 0) {
            throw new RuntimeException("Failed to create the GLFW window");
        }
        GLFW.glfwSetKeyCallback(window, (windowHandle, key, scancode, action, mods) -> {
            switch (action) {
                case GLFW.GLFW_PRESS -> {
                    if (key == exitKey) {
                        GLFW.glfwSetWindowShouldClose(windowHandle, true);
                    }
                    keyPress(key);
                }
                case GLFW.GLFW_RELEASE -> {
                    keyRelease(key);
                }
            }
        });
        GLFW.glfwSetMouseButtonCallback(window, (windowHandle, button, action, mods) -> {
            if (action == GLFW.GLFW_PRESS) {
                mousePress(button);
            } else if (action == GLFW.GLFW_RELEASE) {
                mouseRelease(button);
            }
        });
        GLFW.glfwSetScrollCallback(window, (windowHandle, xOffset, yOffset) -> {
            scroll((float) yOffset);
        });

        if (fullscreen) {
            // Set the window position to the top-left corner of the monitor
            glfwSetWindowPos(window, 0, 0);
        } else {
            // Center the window on the screen
            glfwSetWindowPos(
                    window,
                    (vidMode.width() - width) / 2,
                    (vidMode.height() - height) / 2
            );
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwShowWindow(window);

        GL.createCapabilities();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    public long getWindow() {
        return this.window;
    }

    public void update() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    public void hideCursor(boolean hide) {
        if (hide) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    public void keyPress(int key) {

    }

    public void keyRelease(int key) {

    }

    public void mousePress(int button) {

    }

    public void mouseRelease(int button) {

    }

    public void scroll(float amt) {

    }

    public void setExitShortCut(int key) {
        this.exitKey = key;
    }

    public boolean isWindowOpen() {
        return !glfwWindowShouldClose(window);
    }

    public void exit() {
        glfwFreeCallbacks(window);
        glfwDestroyWindow(window);
        glfwTerminate();
        GLFWErrorCallback err = glfwSetErrorCallback(null);
        if (err != null) {
            err.free();
        }
    }

    public int getWidth() {
        return this.width;
    }

    public int getHeight() {
        return this.height;
    }
}
