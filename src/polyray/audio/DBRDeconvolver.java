package polyray.audio;

import java.util.Random;

// Needs more concrete testing to ensure that it works perfectly for all scenarios and environments
public class DBRDeconvolver {

    public static float[] deconvolve(float[] ref, float[] rec, int iterations, float learningRate) throws DeconvolutionException {
        return gradientDescent(ref, rec, iterations, learningRate);
    }

    public static Object[] deconvolve(float[] ref, float[] rec, int iterations, float learningRate, float volumeThreshold) throws DeconvolutionException {
        float[] W = gradientDescent(ref, rec, iterations, learningRate);
        int count = 0;
        for (int i = 0; i < W.length; i++) {
            if (W[i] > volumeThreshold) {
                count++;
            }
        }
        int[] offsets = new int[count];
        float[] weights = new float[count];
        int idx = 0;
        for (int i = 0; i < W.length; i++) {
            if (W[i] > volumeThreshold) {
                offsets[idx] = i;
                weights[idx] = W[i];
                idx++;
            }
        }
        return new Object[]{weights, offsets};
    }

    // TODO: Implementing this in C++ with SIMD for maximium performance
    private static float[] gradientDescent(float[] V, float[] G, int iterations, float learningRate) throws DeconvolutionException {
        int len = G.length - V.length;
        if(len <= 0) {
            throw new DeconvolutionException("Reference Signal must be shorter than the Target Signal");
        }
        float[] W = new float[len];
        float[] Gp = new float[G.length];

        for (int i = 0; i < G.length; i++) {
            G[i] = -G[i];
        }

        Random rand = new Random();
        for (int i = 0; i < W.length; i++) {
            W[i] = (rand.nextFloat() - 0.5f) * 0.1f;
        }

        learningRate /= W.length;

        System.arraycopy(G, 0, Gp, 0, G.length);
        for (int n = 0; n < W.length; n++) {
            float w = W[n];
            for (int i = 0; i < V.length; i++) {
                Gp[i + n] += V[i] * w;
            }
        }
        float firstLoss = 0.0f;
        for (int i = 0; i < G.length; i++) {
            float diff = Gp[i];
            firstLoss += diff * diff;
        }
        firstLoss /= G.length;
        
        boolean first = true;
        for (int iter = 0; iter < iterations; iter++) {
            System.arraycopy(G, 0, Gp, 0, G.length);
            for (int n = 0; n < W.length; n++) {
                float w = W[n];
                for (int i = 0; i < V.length; i++) {
                    Gp[i + n] += V[i] * w;
                }
            }
            for (int n = 0; n < W.length; n++) {
                float g = 0.0f;
                for (int i = 0; i < V.length; i++) {
                    g += Gp[i + n] * V[i];
                }
                W[n] -= learningRate * g;
            }
            if(first) {
                float loss = 0.0f;
                for (int i = 0; i < G.length; i++) {
                    float diff = Gp[i];
                    loss += diff * diff;
                }
                loss /= G.length;
                if(loss > firstLoss) {
                    throw new DeconvolutionException("Divergence Detected, Learning Rate too High!");
                }
                first = false;
            }
        }
        return W;
    }

    public static class DeconvolutionException extends Exception {

        public DeconvolutionException(String message) {
            super(message);
        }
    }
}
