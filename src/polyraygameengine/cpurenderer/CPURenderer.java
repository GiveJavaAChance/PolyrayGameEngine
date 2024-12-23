package polyraygameengine.cpurenderer;

import java.awt.image.BufferedImage;
import java.awt.image.DataBufferInt;
import java.util.ArrayList;

public class CPURenderer {

    public static final int DEPTH_BUFFER_BIT = 1;
    public static final int COLOR_BUFFER_BIT = 2;
    public static final int NO_BLEND = 3;
    public static final int ALPHA_BLEND = 4;

    private final float[] depthBuffer;
    private final int[] pixels;
    public final BufferedImage render;
    public final int width;
    public final int height;
    private int background = 0xFF000000;
    private int blendMode = 3;

    private long startTime;

    private Shader shader = (color, depth) -> {
        return color;
    };

    private OpenJGL u = null;

    private ArrayList<APIEvent> apiCalls = new ArrayList<>();
    private boolean doCapture;

    public CPURenderer(int width, int height) {
        this.width = width;
        this.height = height;
        this.depthBuffer = new float[width * height];
        for (int i = 0; i < depthBuffer.length; i++) {
            this.depthBuffer[i] = Float.MAX_VALUE;
        }
        this.render = new BufferedImage(width, height, BufferedImage.TYPE_INT_ARGB);
        this.pixels = ((DataBufferInt) this.render.getRaster().getDataBuffer()).getData();
    }

    public void set3DContext(OpenJGL u) {
        this.u = u;
    }

    public void setBackground(int color) {
        this.background = color;
    }

    public void setBlendMode(int mode) {
        this.blendMode = mode;
    }

    public void setShader(Shader shader) {
        this.shader = shader;
    }

    public void setPixel(int x, int y, int color) {
        pixels[x + y * width] = color;
    }

    public int getPixel(int x, int y) {
        return pixels[x + y * width];
    }

    public void fillRect(int x, int y, float z, int width, int height, int color) {
        for (int Y = 0; Y < height; Y++) {
            int py = Y + y;
            if (py < 0) {
                continue;
            } else if (py >= height) {
                break;
            }
            int yi = py * this.width;
            for (int X = 0; X < width; X++) {
                int px = X + x;
                if (px < 0) {
                    continue;
                } else if (px >= width) {
                    break;
                }
                int idx = px + yi;
                if (depthBuffer[idx] < z) {
                    continue;
                }
                pixels[idx] = color;
                depthBuffer[idx] = z;
            }
        }
    }

    public void fillRect(int x, int y, float z00, float z01, float z10, float z11, int width, int height, int color) {
        for (int Y = 0; Y < height; Y++) {
            int py = Y + y;
            float ty = (float) Y / height;
            float ity = 1.0f - ty;
            int yi = py * this.width;
            for (int X = 0; X < width; X++) {
                int px = X + x;
                float tx = (float) X / width;
                float itx = 1.0f - tx;
                int idx = px + yi;
                float z = z00 * itx * ity
                        + z01 * tx * ity
                        + z10 * itx * ty
                        + z11 * tx * ty;
                if (depthBuffer[idx] < z) {
                    continue;
                }
                pixels[idx] = color;
                depthBuffer[idx] = z;
            }
        }
    }

    private float perp(float start, float end, float t) { // Perspective correct interpolation for depth
        if (u == null) {
            return start * end / (end + t * (start - end)) - width / 2.0f;
        }
        return start * end / (end + t * (start - end)) - u.minRendDist;
    }

    public void fillTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2, int color) {
        if (doCapture) {
            TriangleDrawEvent e = new TriangleDrawEvent();
            long time = System.nanoTime();
            e.time = (time - startTime);
            startTime = time;
            e.x0 = x0;
            e.y0 = y0;
            e.z0 = z0;
            e.x1 = x1;
            e.y1 = y1;
            e.z1 = z1;
            e.x2 = x2;
            e.y2 = y2;
            e.z2 = z2;
            e.color = color;
            apiCalls.add(e);
        }
        if ((color >>> 24) == 0) {
            return;
        }
        if (!isVisible(x0, y0, x1, y1, x2, y2)) {
            return; // Triangle is completely outside the viewport
        }
        // Sort vertices by y-coordinate (ascending)
        if (y0 > y1) {
            int tempX = x0, tempY = y0;
            float tempZ = z0;
            x0 = x1;
            y0 = y1;
            z0 = z1;
            x1 = tempX;
            y1 = tempY;
            z1 = tempZ;
        }
        if (y1 > y2) {
            int tempX = x1, tempY = y1;
            float tempZ = z1;
            x1 = x2;
            y1 = y2;
            z1 = z2;
            x2 = tempX;
            y2 = tempY;
            z2 = tempZ;
        }
        if (y0 > y1) {
            int tempX = x0, tempY = y0;
            float tempZ = z0;
            x0 = x1;
            y0 = y1;
            z0 = z1;
            x1 = tempX;
            y1 = tempY;
            z1 = tempZ;
        }

        if (y1 == y2) {
            // Flat-bottom triangle
            fillFlatBottomTriangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
        } else if (y0 == y1) {
            // Flat-top triangle
            fillFlatTopTriangle(x0, y0, z0, x1, y1, z1, x2, y2, z2, color);
        } else {
            // General triangle, split into two triangles
            int y10 = y1 - y0;
            int y20 = y2 - y0;
            float yk = (float) y10 / y20;
            int x3 = (int) (x0 + (x2 - x0) * yk);
            float z3 = z0 + (z2 - z0) * yk;
            fillFlatBottomTriangle(x0, y0, z0, x1, y1, z1, x3, y1, z3, color);
            fillFlatTopTriangle(x1, y1, z1, x3, y1, z3, x2, y2, z2, color);
        }
    }

    public void fillTexturedTriangle(int x0, int y0, float z0, float u0, float v0, int x1, int y1, float z1, float u1, float v1, int x2, int y2, float z2, float u2, float v2, Texture tex) {
        if (doCapture) {
            TextureTriangleDrawEvent e = new TextureTriangleDrawEvent();
            long time = System.nanoTime();
            e.time = (time - startTime);
            startTime = time;
            e.x0 = x0;
            e.y0 = y0;
            e.z0 = z0;
            e.u0 = u0;
            e.v0 = v0;
            e.x1 = x1;
            e.y1 = y1;
            e.z1 = z1;
            e.u1 = u1;
            e.v1 = v1;
            e.x2 = x2;
            e.y2 = y2;
            e.z2 = z2;
            e.u2 = u2;
            e.v2 = v2;
            e.texture = tex;
            apiCalls.add(e);
        }
        if (!isVisible(x0, y0, x1, y1, x2, y2)) {
            return; // Triangle is completely outside the viewport
        }
        // Sort vertices by y-coordinate (ascending)
        if (y0 > y1) {
            int tmpX = x0, tmpY = y0;
            float tmpZ = z0, tmpU = u0, tmpV = v0;
            x0 = x1;
            y0 = y1;
            z0 = z1;
            u0 = u1;
            v0 = v1;
            x1 = tmpX;
            y1 = tmpY;
            z1 = tmpZ;
            u1 = tmpU;
            v1 = tmpV;
        }
        if (y1 > y2) {
            int tmpX = x1, tmpY = y1;
            float tmpZ = z1, tmpU = u1, tmpV = v1;
            x1 = x2;
            y1 = y2;
            z1 = z2;
            u1 = u2;
            v1 = v2;
            x2 = tmpX;
            y2 = tmpY;
            z2 = tmpZ;
            u2 = tmpU;
            v2 = tmpV;
        }
        if (y0 > y1) {
            int tmpX = x0, tmpY = y0;
            float tmpZ = z0, tmpU = u0, tmpV = v0;
            x0 = x1;
            y0 = y1;
            z0 = z1;
            u0 = u1;
            v0 = v1;
            x1 = tmpX;
            y1 = tmpY;
            z1 = tmpZ;
            u1 = tmpU;
            v1 = tmpV;
        }

        if (y1 == y2) {
            // Flat-bottom triangle
            fillFlatBottomTexturedTriangle(x0, y0, z0, u0, v0, x1, y1, z1, u1, v1, x2, y2, z2, u2, v2, tex);
        } else if (y0 == y1) {
            // Flat-top triangle
            fillFlatTopTexturedTriangle(x0, y0, z0, u0, v0, x1, y1, z1, u1, v1, x2, y2, z2, u2, v2, tex);
        } else {
            // General triangle, split into two triangles
            int y10 = y1 - y0;
            int y20 = y2 - y0;
            float yk = (float) y10 / y20;
            int x3 = (int) (x0 + (x2 - x0) * yk);
            float z3 = z0 + (z2 - z0) * yk;
            float u3 = u0 + (u2 - u0) * yk;
            float v3 = v0 + (v2 - v0) * yk;
            fillFlatBottomTexturedTriangle(x0, y0, z0, u0, v0, x1, y1, z1, u1, v1, x3, y1, z3, u3, v3, tex);
            fillFlatTopTexturedTriangle(x1, y1, z1, u1, v1, x3, y1, z3, u3, v3, x2, y2, z2, u2, v2, tex);
        }
    }

    private void fillFlatBottomTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2, int color) {
        int dy = y1 - y0;
        int dx10 = x1 - x0;
        int dx20 = x2 - x0;
        float dz10 = z1 - z0;
        float dz20 = z2 - z0;
        int sy = Math.max(y0, 0);
        int ey = Math.min(y1, height - 1);
        for (int y = sy; y <= ey; y++) {
            float t = (float) (y - y0) / dy;
            int startX = (int) (x0 + t * dx10);
            int endX = (int) (x0 + t * dx20);
            float startZ = z0 + t * dz10;
            float endZ = z0 + t * dz20;
            drawScanline(startX, endX, y, startZ, endZ, color);
        }
    }

    private void fillFlatTopTriangle(int x0, int y0, float z0, int x1, int y1, float z1, int x2, int y2, float z2, int color) {
        int dy = y2 - y0;
        int dx20 = x2 - x0;
        int dx21 = x2 - x1;
        float dz20 = z2 - z0;
        float dz21 = z2 - z1;
        int sy = Math.min(y2, height - 1);
        int ey = Math.max(y0, 0);
        for (int y = sy; y > ey; y--) {
            float t = (float) (y - y0) / dy;
            int startX = (int) (x0 + t * dx20);
            int endX = (int) (x1 + t * dx21);
            float startZ = z0 + t * dz20;
            float endZ = z1 + t * dz21;
            drawScanline(startX, endX, y, startZ, endZ, color);
        }
    }

    private void fillFlatBottomTexturedTriangle(int x0, int y0, float z0, float u0, float v0, int x1, int y1, float z1, float u1, float v1, int x2, int y2, float z2, float u2, float v2, Texture tex) {
        int dy = y1 - y0;
        int dx10 = x1 - x0;
        int dx20 = x2 - x0;
        float dz10 = z1 - z0;
        float dz20 = z2 - z0;
        float du10 = u1 - u0;
        float du20 = u2 - u0;
        float dv10 = v1 - v0;
        float dv20 = v2 - v0;
        int sy = Math.max(y0, 0);
        int ey = Math.min(y1, height - 1);
        for (int y = sy; y <= ey; y++) {
            float t = (float) (y - y0) / dy;
            int startX = (int) (x0 + t * dx10);
            int endX = (int) (x0 + t * dx20);
            float startZ = z0 + t * dz10;
            float endZ = z0 + t * dz20;
            float startU = u0 + t * du10;
            float endU = u0 + t * du20;
            float startV = v0 + t * dv10;
            float endV = v0 + t * dv20;
            drawTexturedScanline(startX, endX, y, startZ, endZ, startU, endU, startV, endV, tex);
        }
    }

    private void fillFlatTopTexturedTriangle(int x0, int y0, float z0, float u0, float v0, int x1, int y1, float z1, float u1, float v1, int x2, int y2, float z2, float u2, float v2, Texture tex) {
        int dy = y2 - y0;
        int dx20 = x2 - x0;
        int dx21 = x2 - x1;
        float dz20 = z2 - z0;
        float dz21 = z2 - z1;
        float du20 = u2 - u0;
        float du21 = u2 - u1;
        float dv20 = v2 - v0;
        float dv21 = v2 - v1;
        int sy = Math.min(y2, height - 1);
        int ey = Math.max(y0, 0);
        for (int y = sy; y > ey; y--) {
            float t = (float) (y - y0) / dy;
            int startX = (int) (x0 + t * dx20);
            int endX = (int) (x1 + t * dx21);
            float startZ = z0 + t * dz20;
            float endZ = z1 + t * dz21;
            float startU = u0 + t * du20;
            float endU = u1 + t * du21;
            float startV = v0 + t * dv20;
            float endV = v1 + t * dv21;
            drawTexturedScanline(startX, endX, y, startZ, endZ, startU, endU, startV, endV, tex);
        }
    }

    private boolean isVisible(int x0, int y0, int x1, int y1, int x2, int y2) {
        return !((x0 < 0 && x1 < 0 && x2 < 0)
                || (x0 >= width && x1 >= width && x2 >= width)
                || (y0 < 0 && y1 < 0 && y2 < 0)
                || (y0 >= height && y1 >= height && y2 >= height));
    }

    private void drawScanline(int x1, int x2, int y, float z1, float z2, int color) {
        if (y < 0 || y >= height) {
            return;
        }
        if (x1 > x2) {
            int tmpX = x1;
            x1 = x2;
            x2 = tmpX;
            float tmpZ = z1;
            z1 = z2;
            z2 = tmpZ;
        }
        int startX = Math.max(0, x1);
        int endX = Math.min(x2, width - 1);
        int len = x2 - x1;

        float zStep = (z2 - z1) / len;

        int yi = y * width;
        int diff = startX - x1;
        float z = z1 + zStep * diff;
        if (blendMode == NO_BLEND) {
            for (int x = startX; x <= endX; x++) {
                int idx = x + yi;
                if (z < depthBuffer[idx]) {
                    depthBuffer[idx] = z;
                    pixels[idx] = shader.compute(color, z);
                }
                z += zStep;
            }
            return;
        }
        int alpha = color >>> 24;
        for (int x = startX; x <= endX; x++) {
            int idx = x + yi;
            if (z < depthBuffer[idx]) {
                depthBuffer[idx] = z;
                int newColor = shader.compute(color, z);
                int srcRGB = newColor & 0x00FFFFFF;
                int destRGB = pixels[idx] & 0x00FFFFFF;
                int outRGB = destRGB + ((((srcRGB & 0xFF00FF) - (destRGB & 0xFF00FF)) * alpha) >> 8) & 0xFF00FF;
                outRGB |= ((destRGB & 0x00FF00) + ((((srcRGB & 0x00FF00) - (destRGB & 0x00FF00)) * alpha) >> 8)) & 0x00FF00;
                pixels[idx] = (0xFF << 24) | outRGB;
            }
            z += zStep;
        }
    }

    private void drawTexturedScanline(int x1, int x2, int y, float z1, float z2, float u1, float u2, float v1, float v2, Texture tex) {
        if (y < 0 || y >= height) {
            return;
        }
        if (x1 > x2) {
            int tmpX = x1;
            x1 = x2;
            x2 = tmpX;
            float tmpZ = z1;
            z1 = z2;
            z2 = tmpZ;
            float tmpU = u1;
            u1 = u2;
            u2 = tmpU;
            float tmpV = v1;
            v1 = v2;
            v2 = tmpV;
        }
        int startX = Math.max(0, x1);
        int endX = Math.min(x2, width - 1);
        int len = x2 - x1;
        int diff = startX - x1;
        float zStep = (z2 - z1) / len;
        float uStep = (u2 - u1) / len;
        float vStep = (v2 - v1) / len;
        float z = z1 + zStep * diff;
        float u = u1 + uStep * diff;
        float v = v1 + vStep * diff;
        int yi = y * width;
        if (blendMode == NO_BLEND) {
            for (int x = startX; x <= endX; x++) {
                int idx = x + yi;
                if (z < depthBuffer[idx]) {
                    depthBuffer[idx] = z;
                    pixels[idx] = shader.compute(tex.getColor(u, v), z);
                }
                z += zStep;
                u += uStep;
                v += vStep;
            }
            return;
        }
        for (int x = startX; x <= endX; x++) {
            int idx = x + yi;
            if (z < depthBuffer[idx]) {
                depthBuffer[idx] = z;
                int color = tex.getColor(u, v);
                int newColor = shader.compute(color, z);
                int alpha = color >>> 24;
                int srcRGB = newColor & 0x00FFFFFF;
                int destRGB = pixels[idx] & 0x00FFFFFF;
                int outRGB = destRGB + ((((srcRGB & 0xFF00FF) - (destRGB & 0xFF00FF)) * alpha) >> 8) & 0xFF00FF;
                outRGB |= ((destRGB & 0x00FF00) + ((((srcRGB & 0x00FF00) - (destRGB & 0x00FF00)) * alpha) >> 8)) & 0x00FF00;
                pixels[idx] = (0xFF << 24) | outRGB;
            }
            z += zStep;
            u += uStep;
            v += vStep;
        }
    }

    public void clear(int mask) {
        if ((mask & 1) == 1) {
            for (int i = 0; i < depthBuffer.length; i++) {
                this.depthBuffer[i] = Float.MAX_VALUE;
            }
        }
        if ((mask & 2) == 2) {
            for (int i = 0; i < pixels.length; i++) {
                pixels[i] = background;
            }
        }
    }

    public float[] getDepthBuffer() {
        return this.depthBuffer;
    }

    public void startFrameCapture() {
        this.doCapture = true;
        this.startTime = System.nanoTime();
    }

    public ArrayList<APIEvent> endFrameCapture() {
        this.doCapture = false;
        ArrayList<APIEvent> events = new ArrayList<>(apiCalls);
        this.apiCalls = new ArrayList<>();
        return events;
    }
}
