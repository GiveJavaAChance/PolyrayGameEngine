package polyray;

import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;

public class TextureUtils {

    public static final Texture createNoiseTexture(int width, int height, float redMin, float redMax, float redPerc, float greenMin, float greenMax, float greenPerc, float blueMin, float blueMax, float bluePerc, float alpha) {
        Texture texture = new Texture(width, height);
        int[] pixels = texture.getData();
        int a = ((int) (alpha * 255.0f)) << 24;
        for (int i = 0; i < pixels.length; i++) {
            int red = 0;
            if (Math.random() < redPerc) {
                red = (int) ((redMin + Math.random() * (redMax - redMin)) * 255.0f);
            }
            int green = 0;
            if (Math.random() < greenPerc) {
                green = (int) ((greenMin + Math.random() * (greenMax - greenMin)) * 255.0f);
            }
            int blue = 0;
            if (Math.random() < bluePerc) {
                blue = (int) ((blueMin + Math.random() * (blueMax - blueMin)) * 255.0f);
            }
            pixels[i] = a | red << 16 | green << 8 | blue;
        }
        return texture;
    }

    public static final Texture createColorTexture(int color) {
        BufferedImage img = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB);
        ((DataBufferInt) img.getRaster().getDataBuffer()).getData()[0] = color;
        return new Texture(img);
    }
}
