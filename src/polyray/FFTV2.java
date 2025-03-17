package polyray;

public class FFTV2 {

    private float[] real, imag;
    private int size;
    private float period, da, db;

    public FFTV2(int size, float period) {
        this.size = size;
        this.period = period;
        this.real = new float[size];
        this.imag = new float[size];
    }

    public void doFFT(int[] data, int range, float minFreq, float intensity, float maxFreq) {
        float length = period / data.length;
        float ang = length * (float) Math.PI * 2.0f;
        float delta = maxFreq - minFreq;
        for (int i = 0; i < size; i++) {
            float f = (float) i / (size - 1.0f);
            float freq = minFreq + f * delta;
            da = (float) Math.sin(ang * freq);
            db = (float) Math.cos(ang * freq);
            calcRotated(data, range, i, intensity * freq);
        }
    }

    public void doLogarithmicFFT(int[] data, int range, float minFreq, float maxFreq, float intensity, float dt, boolean pure) {
        if (dt <= 0.0f) {
            dt = Math.abs(dt) + 0.01f;
        }
        float length = period / data.length;
        float ang = length * (float) Math.PI * 2.0f;
        float delta = maxFreq - minFreq;
        float diff = delta / ((float) Math.pow(2.0f, dt) - 1.0f);
        for (int i = 0; i < size; i++) {
            float f = (float) i / (size - 1.0f);
            float freq = (float) (minFreq + (Math.pow(2.0f, f * dt) - 1.0f) * diff);
            da = (float) Math.sin(ang * freq);
            db = (float) Math.cos(ang * freq);
            if (pure) {
                calcRotated(data, range, i, 1.0f);
            } else {
                calcRotated(data, range, i, intensity * freq);
            }
        }
    }

    private void calcRotated(int[] data, int range, int index, float intensity) {
        float[] start = {1.0f, 0.0f};
        float re = 0.0f, im = 0.0f;
        for (int i = 0; i < data.length; i++) {
            float d = (float) data[i] / range;
            re += start[0] * d;
            im += start[1] * d;
            start = rotate(start);
        }
        real[index] = re / data.length * intensity;
        imag[index] = im / data.length * intensity;
    }

    private float[] rotate(float[] p) {
        return new float[]{p[0] * db - p[1] * da, p[0] * da + p[1] * db};
    }

    public float[] getReal() {
        return real;
    }

    public float[] getImag() {
        return imag;
    }

    public float[] getAmp() {
        float[] out = new float[size];
        for (int i = 0; i < size; i++) {
            float re = real[i];
            float im = imag[i];
            out[i] = (float) Math.sqrt(re * re + im * im);
        }
        return out;
    }
}
