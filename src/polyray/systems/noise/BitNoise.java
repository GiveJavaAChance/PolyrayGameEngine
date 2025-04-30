package polyray.systems.noise;

public class BitNoise {

    private static int hash(int x) {
        x = (x ^ 61) ^ (x >>> 16);
        x *= 9;
        x = x ^ (x >>> 4);
        x *= 0x27d4eb2d;
        x = x ^ (x >>> 15);
        return x;
    }

    private static int hash2D(int x, int y) {
        int pX = x * 1664525 + 1013904223;
        int pY = y * 1664525 + 1013904223;
        pX += pY * 1013904223;
        return hash(pX);
    }

    public static float noise2D(float x, float y) {
        int ix = (int) Math.floor(x);
        int iy = (int) Math.floor(y);
        float fx = x - ix;
        float fy = y - iy;

        int a = hash2D(ix, iy);
        int b = hash2D(ix + 1, iy);
        int c = hash2D(ix, iy + 1);
        int d = hash2D(ix + 1, iy + 1);

        float fadeX = ramp(fx);
        float fadeY = ramp(fy);

        float va  = lerp((a & 0xFFFFFFFFL) / 4294967296.0f,
                (b & 0xFFFFFFFFL) / 4294967296.0f, fadeX) * 2.0f - 1.0f;
        float vb = lerp((c & 0xFFFFFFFFL) / 4294967296.0f,
                (d & 0xFFFFFFFFL) / 4294967296.0f, fadeX) * 2.0f - 1.0f;
        return lerp(va, vb, fadeY);
    }

    private static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    private static float ramp(float v) {
        return 3.0f * v * v - 2.0f * v * v * v;
    }
}
