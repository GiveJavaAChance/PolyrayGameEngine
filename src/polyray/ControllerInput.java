package polyray;

import java.nio.ByteBuffer;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import static org.lwjgl.glfw.GLFW.*;

public final class ControllerInput {

    private static final HashMap<Integer, Boolean> BUTTON_MAP = new HashMap<>();
    private static final HashMap<Integer, Float> JOYSTICK_MAP = new HashMap<>();
    private static final ArrayList<Integer> allControllers = new ArrayList<>(16);
    private static final ArrayList<Integer> controllers = new ArrayList<>();

    public static final int BUTTON_PLAYSTATION_X = 0;
    public static final int BUTTON_PLAYSTATION_O = 1;
    public static final int BUTTON_PLAYSTATION_SQUARE = 2;
    public static final int BUTTON_PLAYSTATION_TRIANGLE = 3;

    public static final int BUTTON_XBOX_A = 0;
    public static final int BUTTON_XBOX_B = 1;
    public static final int BUTTON_XBOX_X = 2;
    public static final int BUTTON_XBOX_Y = 3;
    public static final int BUTTON_XBOX_BACK = 8;

    public static final int BUTTON_L1 = 4;
    public static final int BUTTON_R1 = 5;
    public static final int BUTTON_L2 = 6;
    public static final int BUTTON_R2 = 7;
    public static final int BUTTON_SELECT = 8;
    public static final int BUTTON_START = 9;
    public static final int BUTTON_L3 = 10;
    public static final int BUTTON_R3 = 11;
    public static final int BUTTON_DPAD_UP = 12;
    public static final int BUTTON_DPAD_RIGHT = 13;
    public static final int BUTTON_DPAD_DOWN = 14;
    public static final int BUTTON_DPAD_LEFT = 15;

    public static final int JOYSTICK_LEFT_X_AXIS = 0;
    public static final int JOYSTICK_LEFT_Y_AXIS = 1;
    public static final int JOYSTICK_RIGHT_X_AXIS = 2;
    public static final int JOYSTICK_RIGHT_Y_AXIS = 3;
    public static final int TRIGGER_LEFT = 4;
    public static final int TRIGGER_RIGHT = 5;

    static {
        for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
            allControllers.add(i);
        }
    }

    public static boolean getButton(int controller, int button) {
        return BUTTON_MAP.getOrDefault(controller | (button << 4), false);
    }

    public static float getJoystickPosition(int controller, int joystick) {
        return JOYSTICK_MAP.getOrDefault(controller | (joystick << 4), 0.0f);
    }

    private static void setButton(int controller, int button, boolean active) {
        BUTTON_MAP.put(controller | (button << 4), active);
    }

    private static void setJoystickPosition(int controller, int joystick, float value) {
        JOYSTICK_MAP.put(controller | (joystick << 4), value);
    }

    public static final ArrayList<Integer> getActiveControllers() {
        return controllers;
    }

    public static final void setup() {
        glfwSetJoystickCallback((jid, event) -> {
            if (event == GLFW_CONNECTED) {
                controllers.add(jid);
            } else if (event == GLFW_DISCONNECTED) {
                controllers.remove((Integer) jid);
            }
        });
        for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
            if (glfwJoystickPresent(i)) {
                controllers.add(i);
            }
        }
    }

    public static void updateControllers() {
        ArrayList<Integer> all = new ArrayList<>(allControllers);
        all.removeAll(controllers);
        for (int c : all) {
            if (glfwJoystickPresent(c)) {
                controllers.add(c);
            }
        }
        for (int controller : controllers) {
            if (glfwJoystickPresent(controller)) {
                ByteBuffer buttons = glfwGetJoystickButtons(controller);
                if (buttons != null) {
                    for (int i = 0; i < buttons.limit(); i++) {
                        setButton(controller, i, buttons.get(i) == GLFW_PRESS);
                    }
                }
                FloatBuffer axes = glfwGetJoystickAxes(controller);
                if (axes != null) {
                    for (int i = 0; i < axes.limit(); i++) {
                        setJoystickPosition(controller, i, axes.get(i));
                    }
                }
            } else {
                controllers.remove((Integer) controller);
            }
        }
    }
}
