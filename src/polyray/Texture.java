package polyray;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.awt.image.ColorModel;
import java.awt.image.DataBuffer;
import java.awt.image.DataBufferInt;
import java.awt.image.DirectColorModel;
import java.awt.image.Raster;
import java.awt.image.SampleModel;
import java.awt.image.SinglePixelPackedSampleModel;
import java.awt.image.WritableRaster;

public class Texture {

    private final BufferedImage texture;
    private final int width;
    private final int height;

    public Texture(BufferedImage image) {
        this.width = image.getWidth();
        this.height = image.getHeight();
        int[] rgba = new int[width * height];
        DataBufferInt dataBuffer = new DataBufferInt(rgba, rgba.length);
        int[] bitMasks = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
        SampleModel sampleModel = new SinglePixelPackedSampleModel(DataBuffer.TYPE_INT, width, height, bitMasks);
        WritableRaster raster = Raster.createWritableRaster(sampleModel, dataBuffer, null);
        ColorModel colorModel = new DirectColorModel(32, bitMasks[0], bitMasks[1], bitMasks[2], bitMasks[3]);
        this.texture = new BufferedImage(colorModel, raster, false, null);
        Graphics2D g = this.texture.createGraphics();
        g.drawImage(image, 0, 0, null);
        g.dispose();
    }

    public Texture(int width, int height) {
        this.width = width;
        this.height = height;
        int[] rgba = new int[width * height];
        DataBufferInt dataBuffer = new DataBufferInt(rgba, rgba.length);
        int[] bitMasks = {0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000};
        SampleModel sampleModel = new SinglePixelPackedSampleModel(DataBuffer.TYPE_INT, width, height, bitMasks);
        WritableRaster raster = Raster.createWritableRaster(sampleModel, dataBuffer, null);
        ColorModel colorModel = new DirectColorModel(32, bitMasks[0], bitMasks[1], bitMasks[2], bitMasks[3]);
        this.texture = new BufferedImage(colorModel, raster, false, null);
    }

    public BufferedImage getTexture() {
        return this.texture;
    }

    public int[] getData() {
        return ((DataBufferInt) texture.getRaster().getDataBuffer()).getData();
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    public Graphics2D createGraphics() {
        return texture.createGraphics();
    }
}
