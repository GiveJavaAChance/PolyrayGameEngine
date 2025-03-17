package polyray.audio;

import java.io.BufferedInputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

public class DBREffect implements Effect {

    private final float[] reverb;
    private final int[] reverbTmp;

    private final int samplesPerBuffer;
    private final float[] volumes;
    private final int[] delays;

    private DBREffect(int bufferSize, float[] volumes, int[] delays, int maxDelay) {
        this.reverb = new float[maxDelay + bufferSize];
        this.reverbTmp = new int[bufferSize];
        this.samplesPerBuffer = delays.length;
        this.volumes = volumes;
        this.delays = delays;
    }

    public DBREffect(int samplesPerBuffer, int maxDelay, int bufferSize) {
        this.reverb = new float[maxDelay + bufferSize];
        this.reverbTmp = new int[bufferSize];
        this.samplesPerBuffer = samplesPerBuffer;
        volumes = new float[samplesPerBuffer];
        delays = new int[samplesPerBuffer];
        for (int i = 0; i < samplesPerBuffer; i++) {
            double r = Math.pow(Math.random(), 2.0d);
            volumes[i] = (float) (1.0d - r);
            delays[i] = (int) (r * maxDelay);
        }
    }

    public DBREffect(int samplesPerBuffer, int maxDelay, int bufferSize, double power) {
        this.reverb = new float[maxDelay + bufferSize];
        this.reverbTmp = new int[bufferSize];
        this.samplesPerBuffer = samplesPerBuffer;
        volumes = new float[samplesPerBuffer];
        delays = new int[samplesPerBuffer];
        for (int i = 0; i < samplesPerBuffer; i++) {
            double r = Math.pow(Math.random(), power);
            volumes[i] = (float) (1.0d - r);
            delays[i] = (int) (r * maxDelay);
        }
    }

    public static final DBREffect loadReverb(String name, float delayMul, float volumeMul, int bufferSize) {
        try ( BufferedInputStream in = new BufferedInputStream(DBREffect.class.getResourceAsStream(name))) {
            byte[] bytes = in.readAllBytes();
            FloatBuffer floats = ByteBuffer.wrap(bytes).order(ByteOrder.BIG_ENDIAN).asFloatBuffer();
            int numFloats = bytes.length >> 3;
            float[] volumes = new float[numFloats];
            int[] delays = new int[numFloats];
            int maxDelay = 0;
            for (int i = 0; i < numFloats; i++) {
                volumes[i] = floats.get() * volumeMul;
                int delay = (int) (floats.get() * delayMul);
                delays[i] = delay;
                if (delay > maxDelay) {
                    maxDelay = delay;
                }
            }
            return new DBREffect(bufferSize, volumes, delays, maxDelay);
        } catch (IOException e) {
        }
        return null;
    }

    private void computeReverb(int[] channel) {
        for (int i = 0; i < samplesPerBuffer; i++) {
            int idx = delays[i];
            float vol = volumes[i];
            for (int j = 0; j < channel.length; j++) {
                reverb[j + idx] += channel[j] * vol;
            }
        }
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) reverb[i];
        }
        System.arraycopy(reverb, channel.length, reverb, 0, reverb.length - channel.length);
        for (int i = reverb.length - channel.length; i < reverb.length; i++) {
            reverb[i] = 0.0f;
        }
        System.arraycopy(channel, 0, reverbTmp, 0, channel.length);
    }

    @Override
    public void computeRight(int[] channel) {
        computeReverb(channel);
    }

    @Override
    public void computeLeft(int[] channel) {
        System.arraycopy(reverbTmp, 0, channel, 0, channel.length);
    }

    @Override
    public void onStart() {

    }

    @Override
    public void onFinnish() {

    }
}
