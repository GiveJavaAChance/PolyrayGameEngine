package polyray.systems.noise;

// "Inigo Quilez Noise"
public class Noise {

    private static final double INVERSE_PI = 1.0d / Math.PI;

    public static double noiseND(double... coords) {
        int n = coords.length;
        double[] iCoords = new double[n];
        double[] fCoords = new double[n];
        double[] u = new double[n];
        for (int i = 0; i < n; i++) {
            iCoords[i] = Math.floor(coords[i]);
            fCoords[i] = coords[i] - iCoords[i];
            u[i] = ramp(fCoords[i]);
        }
        int numCorners = 1 << n;
        double noiseValue = 0.0d;
        for (int corner = 0; corner < numCorners; corner++) {
            double weight = 1.0d;
            double[] cornerCoords = new double[n];
            for (int i = 0; i < n; i++) {
                if ((corner & (1 << i)) == 0) {
                    cornerCoords[i] = iCoords[i];
                    weight *= (1 - u[i]);
                } else {
                    cornerCoords[i] = iCoords[i] + 1.0d;
                    weight *= u[i];
                }
            }
            noiseValue += weight * getValND(cornerCoords);
        }
        return noiseValue;
    }

    private static double getValND(double... coords) {
        double val = 1.0d;
        for (double c : coords) {
            double u = 50.0d * fract(c * INVERSE_PI);
            val *= u;
        }
        return 2.0d * fract( val) - 1.0d;
    }

    public static double noise2D(double x, double y) {
        double ix = (double) Math.floor(x);
        double iy = (double) Math.floor(y);
        double u = ramp(x - ix);
        double v = ramp(y - iy);
        double a = getVal2D(ix, iy);
        double b = getVal2D(ix + 1.0d, iy);
        double c = getVal2D(ix, iy + 1.0d);
        double d = getVal2D(ix + 1.0d, iy + 1.0d);
        return a + (b - a) * u + (c - a) * v + (a - b - c + d) * u * v;
    }

    private static double getVal2D(double x, double y) {
        double u = 50.0d * fract(x * INVERSE_PI);
        double v = 50.0d * fract(y * INVERSE_PI);
        return 2.0d * fract(u * v * (u + v)) - 1.0d;
    }

    private static double fract(double v) {
        return v - (double) Math.floor(v);
    }

    private static double ramp(double v) {
        return 3.0d * v * v - 2.0d * v * v * v;
    }
}
