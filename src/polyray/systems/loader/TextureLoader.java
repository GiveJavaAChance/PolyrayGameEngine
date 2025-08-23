package polyray.systems.loader;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.IOException;
import java.util.HashMap;
import javax.imageio.ImageIO;
import static org.lwjgl.opengl.GL11.GL_RGBA8;
import polyray.GLTexture.GLTexture2D;
import polyray.GLTexture.GLTextureArray;
import polyray.ResourceLoader;
import polyray.Texture;

public class TextureLoader {

    private static final HashMap<String, GLTexture2D> textures = new HashMap<>();
    private static final HashMap<String, GLTextureArray> textureArrays = new HashMap<>();

    public static final GLTextureArray loadTextureArray(String name, int numModelsX, int numModelsY, int interFormat, boolean interpolate, int wrapMode) {
        GLTextureArray tex = textureArrays.get(name);
        if (tex == null) {
            int numModels = numModelsX * numModelsY;
            try {
                BufferedImage playerSegment = ImageIO.read(ResourceLoader.getLoader().getResourceAsStream(name));
                int width = playerSegment.getWidth();
                int height = playerSegment.getHeight();
                int modelWidth = width / numModelsX;
                int modelHeight = height / numModelsY;
                tex = new GLTextureArray(modelWidth, modelHeight, numModels, interFormat);
                tex.setInterpolation(interpolate);
                tex.setWrapMode(wrapMode);
                for (int y = 0; y < numModelsY; y++) {
                    int yi = y * numModelsX;
                    for (int x = 0; x < numModelsX; x++) {
                        int idx = x + yi;
                        Texture t = new Texture(modelWidth, modelHeight);
                        Graphics2D g = t.createGraphics();
                        g.drawImage(playerSegment, -x * modelWidth, -y * modelHeight, null);
                        g.dispose();
                        tex.setLayerData(idx, t);
                    }
                }
            } catch (IOException e) {
            }
            textureArrays.put(name, tex);
        }
        return tex;
    }

    public static final GLTextureArray loadStaticTextureArray(String name, int modelWidth, int modelHeight, int interFormat, boolean interpolate, int wrapMode) {
        GLTextureArray tex = textureArrays.get(name);
        if (tex == null) {
            try {
                BufferedImage playerSegment = ImageIO.read(ResourceLoader.getLoader().getResourceAsStream(name));
                int width = playerSegment.getWidth();
                int height = playerSegment.getHeight();
                int numModelsX = width / modelWidth;
                int numModelsY = height / modelHeight;
                int numModels = numModelsX * numModelsY;
                tex = new GLTextureArray(modelWidth, modelHeight, numModels, interFormat);
                tex.setInterpolation(interpolate);
                tex.setWrapMode(wrapMode);
                for (int y = 0; y < numModelsY; y++) {
                    int yi = y * numModelsX;
                    for (int x = 0; x < numModelsX; x++) {
                        int idx = x + yi;
                        Texture t = new Texture(modelWidth, modelHeight);
                        Graphics2D g = t.createGraphics();
                        g.drawImage(playerSegment, -x * modelWidth, -y * modelHeight, null);
                        g.dispose();
                        tex.setLayerData(idx, t);
                    }
                }
            } catch (IOException e) {
            }
            textureArrays.put(name, tex);
        }
        return tex;
    }

    public static final GLTexture2D loadTexture(String name) throws IOException {
        GLTexture2D tex = textures.get(name);
        if (tex == null) {
            tex = new GLTexture2D(new Texture(ImageIO.read(ResourceLoader.getLoader().getResourceAsStream(name))), GL_RGBA8);
            tex.setInterpolation(true);
            textures.put(name, tex);
        }
        return tex;
    }
}
