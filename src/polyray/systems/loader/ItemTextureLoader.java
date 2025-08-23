package polyray.systems.loader;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import javax.imageio.ImageIO;
import static org.lwjgl.opengl.GL11.GL_RGBA8;
import polyray.GLTexture.GLTextureArray;
import polyray.Texture;

public class ItemTextureLoader {

    private static final ArrayList<Texture> layers = new ArrayList<>();
    private static final HashMap<String, Integer> baseIndices = new HashMap<>();
    private static boolean set = false;
    private static int W, H;
    
    public static final void setTextureSize(int width, int height) {
        if(set) {
            return;
        }
        W = width;
        H = height;
        set = true;
    }

    public static final int loadTextures(String name, ClassLoader loader) {
        if(!set) {
            throw new RuntimeException("Texture Size not Set!");
        }
        Integer i = baseIndices.get(name);
        if (i == null) {
            i = layers.size();
            baseIndices.put(name, i);
            try {
                BufferedImage itemTextures = ImageIO.read(loader.getResourceAsStream(name));
                int width = itemTextures.getWidth();
                int height = itemTextures.getHeight();
                int numModelsX = width / W;
                int numModelsY = height / H;
                for (int y = 0; y < numModelsY; y++) {
                    for (int x = 0; x < numModelsX; x++) {
                        Texture t = new Texture(W, H);
                        Graphics2D g = t.createGraphics();
                        g.drawImage(itemTextures, -x * W, -y * H, null);
                        g.dispose();
                        layers.add(t);
                    }
                }
            } catch (IOException e) {
            }
        }
        return i;
    }

    public static final GLTextureArray compile() {
        if(!set) {
            throw new RuntimeException("Texture Size not Set!");
        }
        GLTextureArray tex = new GLTextureArray(W, H, layers.size(), GL_RGBA8);
        for (int i = 0; i < layers.size(); i++) {
            tex.setLayerData(i, layers.get(i));
        }
        System.out.println("Item Texture Count: " + layers.size());
        set = false;
        return tex;
    }
}
