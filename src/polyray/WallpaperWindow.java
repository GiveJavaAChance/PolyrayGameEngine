package polyray;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import org.lwjgl.opengl.GL;
import static org.lwjgl.opengl.GL11.GL_BLEND;
import static org.lwjgl.opengl.GL11.GL_DEPTH_TEST;
import static org.lwjgl.opengl.GL11.GL_ONE_MINUS_SRC_ALPHA;
import static org.lwjgl.opengl.GL11.GL_SRC_ALPHA;
import static org.lwjgl.opengl.GL11.glBlendFunc;
import static org.lwjgl.opengl.GL11.glDisable;
import static org.lwjgl.opengl.GL11.glEnable;

public class WallpaperWindow {
    
    static {
        try {
            File tempFile = File.createTempFile("WallpaperWindow", ".dll");
            tempFile.deleteOnExit();
            try ( FileOutputStream out = new FileOutputStream(tempFile);  InputStream in = WallpaperWindow.class.getResourceAsStream("WallpaperWindow.dll")) {
                byte[] buffer = new byte[4096];
                int bytesRead;
                while ((bytesRead = in.read(buffer)) != -1) {
                    out.write(buffer, 0, bytesRead);
                }
            }
            System.load(tempFile.getAbsolutePath());
        } catch (IOException e) {
        }
    }

    private final long window;

    public WallpaperWindow() {
        this.window = createWindow();
        GL.createCapabilities();
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    public void update() {
        swapBuffers(window);
    }
    
    public void exit() {
        deleteWindow(window);
    }

    private static native long createWindow();

    private static native void swapBuffers(long window);
    
    private static native void deleteWindow(long window);
}
