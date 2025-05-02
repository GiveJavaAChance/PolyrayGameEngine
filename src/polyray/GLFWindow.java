package polyray;

import java.awt.DisplayMode;
import java.awt.GraphicsDevice;
import java.awt.GraphicsEnvironment;
import java.util.HashMap;
import static org.lwjgl.glfw.Callbacks.glfwFreeCallbacks;
import static org.lwjgl.glfw.GLFW.*;
import org.lwjgl.glfw.GLFWErrorCallback;
import org.lwjgl.opengl.GL;
import static org.lwjgl.opengl.GL11.*;

public class GLFWindow {

    private int width, height;
    private String name = "";
    private long window;
    private int exitKey = GLFW_KEY_Q;

    private final int numEffects = 20;
    private int effect = numEffects - 1;
    private int effectKey = GLFW_KEY_G;

    private int button = -1;
    private float mx, my;

    public GLFWindow(String name) {
        this.name = name;
    }

    public void createFrame(int width, int height, boolean title, boolean fullscreen, boolean exclusive) {
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
        if (fullscreen && exclusive) {
            long monitor = glfwGetPrimaryMonitor(); // Get the primary monitor
            window = glfwCreateWindow(this.width, this.height, name, monitor, 0l);
            if (window == 0) {
                throw new RuntimeException("Failed to create the GLFW window");
            }
        } else {
            int titleHint = title ? GLFW_TRUE : GLFW_FALSE;
            glfwWindowHint(GLFW_RESIZABLE, titleHint);
            glfwWindowHint(GLFW_DECORATED, titleHint);
            glfwWindowHint(GLFW_MAXIMIZED, titleHint);

            window = glfwCreateWindow(this.width, this.height, name, 0l, 0l);

            if (window == 0) {
                throw new RuntimeException("Failed to create the GLFW window");
            }

            if (fullscreen && !title) {
                glfwSetWindowPos(window, 0, 0);
            }
        }

        glfwSetKeyCallback(window, (windowHandle, key, scancode, action, mods) -> {
            switch (action) {
                case GLFW_PRESS -> {
                    if (key == exitKey) {
                        glfwSetWindowShouldClose(windowHandle, true);
                    } else if (key == effectKey) {
                        effect = (effect + 1) % numEffects;
                        effectChanged(effect);
                    }
                    Input.setKey(key, true);
                    keyPress(key);
                }
                case GLFW_RELEASE -> {
                    Input.setKey(key, false);
                    keyRelease(key);
                }
            }
        });
        glfwSetMouseButtonCallback(window, (windowHandle, b, action, mods) -> {
            double[] xPos = new double[1];
            double[] yPos = new double[1];
            glfwGetCursorPos(windowHandle, xPos, yPos);
            float x = (float) xPos[0];
            float y = this.height - (float) yPos[0];
            if (action == GLFW_PRESS) {
                mousePress(x, y, b);
                button = b;
            } else if (action == GLFW_RELEASE) {
                mouseRelease(x, y, b);
                button = -1;
            }
        });
        glfwSetCursorPosCallback(window, (windowHandle, xpos, ypos) -> {
            float x = (float) xpos;
            float y = this.height - (float) ypos;
            if (button != -1) {
                mouseDrag(mx, my, x, y, button);
            } else {
                mouseMove(x, y);
            }
            mx = x;
            my = y;
        });

        glfwSetScrollCallback(window, (windowHandle, xOffset, yOffset) -> {
            double[] xPos = new double[1];
            double[] yPos = new double[1];
            glfwGetCursorPos(windowHandle, xPos, yPos);
            float x = (float) xPos[0];
            float y = this.height - (float) yPos[0];
            scroll(x, y, (float) yOffset);
        });
        glfwSetWindowSizeCallback(window, (windowHandle, newWidth, newHeight) -> {
            this.width = newWidth;
            this.height = newHeight;
            glViewport(0, 0, width, height);
            windowResized(newWidth, newHeight);
        });
        glfwSetFramebufferSizeCallback(window, (windowHandle, fbWidth, fbHeight) -> {
            this.width = fbWidth;
            this.height = fbHeight;
            glViewport(0, 0, fbWidth, fbHeight);
            windowResized(fbWidth, fbHeight);
        });
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        glfwShowWindow(window);
        glfwFocusWindow(window);

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

    public Vector2d getMousePos() {
        double[] xPos = new double[1];
        double[] yPos = new double[1];
        glfwGetCursorPos(window, xPos, yPos);
        return new Vector2d(xPos[0], yPos[0]);
    }

    public void keyPress(int key) {

    }

    public void keyRelease(int key) {

    }

    public void mousePress(float x, float y, int button) {

    }

    public void mouseRelease(float x, float y, int button) {

    }

    public void mouseMove(float x, float y) {

    }

    public void mouseDrag(float x0, float y0, float x1, float y1, int button) {

    }

    public void scroll(float x, float y, float amt) {

    }

    public void effectChanged(int effect) {

    }

    public void windowResized(int width, int height) {

    }

    public void setExitShortCut(int key) {
        this.exitKey = key;
    }

    public void setEffectKey(int key) {
        this.effectKey = key;
    }

    public boolean isWindowOpen() {
        return !glfwWindowShouldClose(window);
    }

    public boolean isWindowFocused() {
        return glfwGetWindowAttrib(window, GLFW_FOCUSED) == GLFW_TRUE;
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
