package polyray;

public class FFTV2 {

    private final float[] real;
    private final float[] imag;
    private final int size;
    private final float period;

    public FFTV2(int size, float period) {
        this.size = size;
        this.period = period;
        this.real = new float[size];
        this.imag = new float[size];
    }

    public void doFFT(int[] data, int range, float minFreq, float intensity, float maxFreq) {
        float[] dat = new float[data.length];
        for (int i = 0; i < data.length; i++) {
            dat[i] = (float) data[i] / range / data.length;
        }

        float length = period / data.length;
        float ang = length * (float) Math.PI * 2.0f;
        float delta = maxFreq - minFreq;
        for (int i = 0; i < size; i++) {
            float f = (float) i / (size - 1.0f);
            float freq = minFreq + f * delta;
            float a = ang * freq;
            float sin = (float) Math.sin(a);
            float cos = (float) Math.cos(a);
            float x = 1.0f;
            float xt;
            float y = 0.0f;
            float re = 0.0f, im = 0.0f;
            for (int j = 0; j < dat.length; j++) {
                float d = dat[j];
                re += x * d;
                im += y * d;
                xt = x * cos - y * sin;
                y = x * sin + y * cos;
                x = xt;
            }
            float I = intensity * freq;
            real[i] = re * I;
            imag[i] = im * I;
        }
    }

    public void doLogarithmicFFT(int[] data, int range, float minFreq, float maxFreq, float intensity, float dt, boolean pure) {
        if (dt <= 0.0f) {
            dt = Math.abs(dt) + 0.01f;
        }
        float[] dat = new float[data.length];
        for (int i = 0; i < data.length; i++) {
            dat[i] = (float) data[i] / range / data.length;
        }

        float length = period / data.length;
        float ang = length * (float) Math.PI * 2.0f;
        float delta = maxFreq - minFreq;
        float diff = delta / ((float) Math.pow(2.0f, dt) - 1.0f);
        for (int i = 0; i < size; i++) {
            float f = (float) i / (size - 1.0f);
            float freq = (float) (minFreq + (Math.pow(2.0f, f * dt) - 1.0f) * diff);
            float a = ang * freq;
            float sin = (float) Math.sin(a);
            float cos = (float) Math.cos(a);
            float x = 1.0f;
            float xt;
            float y = 0.0f;
            float re = 0.0f, im = 0.0f;
            for (int j = 0; j < dat.length; j++) {
                float d = dat[j];
                re += x * d;
                im += y * d;
                xt = x * cos - y * sin;
                y = x * sin + y * cos;
                x = xt;
            }
            float I = pure ? 1.0f : intensity * freq;
            real[i] = re * I;
            imag[i] = im * I;
        }
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