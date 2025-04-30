package polyray.systems.noise;

import polyray.Vector2f;

public class VoronoiPath {

    public static float noise2D(float x, float y) {
        float ix = (float) Math.floor(x - 0.5f);
        float iy = (float) Math.floor(y - 0.5f);
        
        float d1 = 10.0f, d2 = 10.0f, d3 = 10.0f;
        float xOff = BitNoise.noise2D(x * 3.0f + 17.2876f, y * 3.0f + 27.2876f) * 0.3f;
        float yOff = BitNoise.noise2D(x * 3.0f - 15.2876f, y * 3.0f + 6.2876f) * 0.3f;
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                Vector2f offset = getCellOffset(ix + dx + xOff, iy + dy + yOff);
                float dist = dst(x - (ix + dx + offset.x), y - (iy + dy + offset.y));
                if (dist < d1) {
                    d3 = d2;
                    d2 = d1;
                    d1 = dist;
                } else if (dist < d2) {
                    d3 = d2;
                    d2 = dist;
                } else if (dist < d3) {
                    d3 = dist;
                }
            }
        }

        float range = 0.01f;
        float a = (float) Math.abs(d1 - d2);
        float b = (float) Math.abs(d2 - d3);
        if (a > range) {
            return 0.0f;
        }
        if (b <= range) {
            float t = Math.min(a, b) / range;
            return falloff(t);
        }
        float t = a / range;
        return falloff(t);
    }

    private static float falloff(float x) {
        return 1.0f - x * x * x * x * x;
    }

    private static float dst(float x, float y) {
        return x * x + y * y;
    }

    private static Vector2f getCellOffset(float x, float y) {
        return new Vector2f(BitNoise.noise2D(x + 168.1876f, y + 1675.12876f) * 0.5f + 0.5f, BitNoise.noise2D(x + 9817.1876f, y + 5674.2685f) * 0.5f + 0.5f);
    }
}
