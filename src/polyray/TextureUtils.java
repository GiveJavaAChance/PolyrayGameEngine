package polyray;

import java.awt.Font;
import java.awt.Graphics2D;
import java.awt.Shape;
import java.awt.font.FontRenderContext;
import java.awt.font.GlyphVector;
import java.awt.geom.AffineTransform;
import java.awt.geom.Rectangle2D;
import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import polyray.GLTexture.GLTextureArray;

public class TextureUtils {

    public static Texture createNoiseTexture(int width, int height, float redMin, float redMax, float redPerc, float greenMin, float greenMax, float greenPerc, float blueMin, float blueMax, float bluePerc, float alpha) {
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

    public static Texture createColorTexture(int color) {
        BufferedImage img = new BufferedImage(1, 1, BufferedImage.TYPE_INT_ARGB);
        ((DataBufferInt) img.getRaster().getDataBuffer()).getData()[0] = color;
        return new Texture(img);
    }

    public static Texture renderMonospacedFontBitmap(Font font, char startingChar, int aliasingQuality) {
        int gridX = 10;
        int gridY = 10;

        int maxGlyphWidth = 0;
        int maxGlyphHeight = 0;
        FontRenderContext frc = new FontRenderContext(null, false, false);

        for (char ch = startingChar, i = 0; i < gridX * gridY; ch++, i++) {
            GlyphVector gv = font.createGlyphVector(frc, new char[]{ch});
            Shape shape = gv.getGlyphOutline(0);
            Rectangle2D bounds = shape.getBounds2D();
            maxGlyphWidth = Math.max(maxGlyphWidth, (int) Math.ceil(bounds.getWidth()));
            maxGlyphHeight = Math.max(maxGlyphHeight, (int) Math.ceil(bounds.getHeight()));
        }

        int cellWidth = maxGlyphWidth;
        int cellHeight = maxGlyphHeight;

        int imageWidth = gridX * cellWidth;
        int imageHeight = gridY * cellHeight;

        BufferedImage bitmap = new BufferedImage(imageWidth, imageHeight, BufferedImage.TYPE_INT_ARGB);
        int[] pixels = ((DataBufferInt) bitmap.getRaster().getDataBuffer()).getData();

        char ch = startingChar;
        for (int y = 0; y < gridY; y++) {
            for (int x = 0; x < gridX; x++) {
                GlyphVector gv = font.createGlyphVector(frc, new char[]{ch});
                Shape shape = gv.getGlyphOutline(0);
                Rectangle2D bounds = shape.getBounds2D();

                double offsetX = (cellWidth - bounds.getWidth()) * 0.5d - bounds.getX();
                double offsetY = (cellHeight - bounds.getHeight()) * 0.5d - bounds.getY();

                int cx = x * cellWidth;
                int cy = y * cellHeight;

                Shape translated = AffineTransform.getTranslateInstance(cx + offsetX, cy + offsetY).createTransformedShape(shape);

                rasterShape(pixels, imageWidth, translated, cx, cy, cellWidth, cellHeight, aliasingQuality);
                ch++;
            }
        }
        return new Texture(bitmap);
    }

    public static GLTextureArray createBitmapTexture(Font font, char startingChar, int aliasingQuality, int interFormat, boolean interpolate, int wrapMode) {
        Texture bitmap = renderMonospacedFontBitmap(font, startingChar, aliasingQuality);
        int numModels = 100;
        int width = bitmap.getWidth();
        int height = bitmap.getHeight();
        int modelWidth = width / 10;
        int modelHeight = height / 10;
        GLTextureArray tex = new GLTextureArray(modelWidth, modelHeight, numModels, interFormat);
        tex.setInterpolation(interpolate);
        tex.setWrapMode(wrapMode);
        for (int y = 0; y < 10; y++) {
            int yi = y * 10;
            for (int x = 0; x < 10; x++) {
                int idx = x + yi;
                Texture t = new Texture(modelWidth, modelHeight);
                Graphics2D g = t.createGraphics();
                g.drawImage(bitmap.getTexture(), -x * modelWidth, -y * modelHeight, null);
                g.dispose();
                tex.setLayerData(idx, t);
            }
        }
        return tex;
    }

    private static void rasterShape(int[] pixels, int width, Shape shape, int cellX, int cellY, int cellWidth, int cellHeight, int sampleSize) {
        for (int y = 0; y < cellHeight; y++) {
            int yi = (cellY + y) * width;
            double py = cellY + y;
            for (int x = 0; x < cellWidth; x++) {
                int hits = 0;
                double px = cellX + x;
                for (int dy = 0; dy < sampleSize; dy++) {
                    double sy = py + (dy + 0.5d) / sampleSize;
                    for (int dx = 0; dx < sampleSize; dx++) {
                        double sx = px + (dx + 0.5d) / sampleSize;
                        if (shape.contains(sx, sy)) {
                            hits++;
                        }
                    }
                }
                int a = 0xFF * hits / sampleSize / sampleSize;
                pixels[cellX + x + yi] = a << 24 | 0x00FFFFFF;
            }
        }
    }
}
