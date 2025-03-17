package polyray.audio;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import javax.sound.sampled.AudioFormat;

public class SoundEffector {

    private HashMap<Integer, Effect> effects = new HashMap<>();
    private ArrayList<Integer> keys = new ArrayList<>();
    private final int numChannels;
    private final int numBytes;
    private int[] rightChannel, leftChannel;

    public SoundEffector(int numChannels, int frameSize) {
        this.numChannels = numChannels;
        this.numBytes = frameSize / this.numChannels;
    }

    public void addEffect(Effect e, int pos) {
        effects.put(pos, e);
        if (!keys.contains(pos)) {
            keys.add(pos);
        }
        Collections.sort(keys);
    }

    public byte[] nextBuffer(byte[] buffer, float time, boolean swapStereo) {
        int len = buffer.length / numBytes / numChannels;
        if (numChannels == 2) {
            this.rightChannel = new int[len];
            this.leftChannel = new int[len];

            // Decode
            int pos = 0;
            for (int j = 0; j < buffer.length; j += 4) {
                short sampleLeft = (short) ((buffer[j + 1] << 8) | (buffer[j] & 0xFF));

                short sampleRight = (short) ((buffer[j + 3] << 8) | (buffer[j + 2] & 0xFF));
                if (swapStereo) {
                    rightChannel[pos] = sampleRight;
                    leftChannel[pos] = sampleLeft;
                } else {
                    rightChannel[pos] = sampleLeft;
                    leftChannel[pos] = sampleRight;
                }
                pos++;
            }

            // Apply effects
            for (int key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.computeLeft(leftChannel);
                effect.onFinnish();
            }
            limit(rightChannel);
            limit(leftChannel);

            // Encode
            byte[] audioDataBack = new byte[len * 4];
            pos = 0;
            for (int j = 0; j < len; j++) {
                short sampleRight = (short) rightChannel[j];
                short sampleLeft = (short) leftChannel[j];

                byte byte1 = (byte) (sampleRight & 0xFF);
                byte byte2 = (byte) ((sampleRight >> 8) & 0xFF);
                byte byte3 = (byte) (sampleLeft & 0xFF);
                byte byte4 = (byte) ((sampleLeft >> 8) & 0xFF);

                audioDataBack[pos] = byte1;
                audioDataBack[pos + 1] = byte2;
                audioDataBack[pos + 2] = byte3;
                audioDataBack[pos + 3] = byte4;
                pos += 4;
            }
            return audioDataBack;
        } else if (numChannels == 1) {
            this.rightChannel = new int[len];

            // Decode
            int pos = 0;
            for (int j = 0; j < buffer.length; j += 2) {
                short sample = (short) ((buffer[j + 1] << 8) | (buffer[j] & 0xFF));
                rightChannel[pos] = sample;
                pos++;
            }

            // Apply effects
            for (int key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.onFinnish();
            }
            limit(rightChannel);

            // Encode
            byte[] audioDataBack = new byte[len * 2];
            pos = 0;
            for (int j = 0; j < len; j++) {
                short sampleRight = (short) rightChannel[j];

                byte byte1 = (byte) (sampleRight & 0xFF);
                byte byte2 = (byte) ((sampleRight >> 8) & 0xFF);

                audioDataBack[pos] = byte1;
                audioDataBack[pos + 1] = byte2;
                pos += 2;
            }
            return audioDataBack;
        }
        return null;
    }

    public byte[] nextBuffer(int[] buffer, float time) {
        int len = buffer.length;
        if (numChannels == 2) {
            this.rightChannel = buffer;
            this.leftChannel = new int[len];
            System.arraycopy(buffer, 0, this.leftChannel, 0, len);

            // Apply effects
            for (int key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.computeLeft(leftChannel);
                effect.onFinnish();
            }
            limit(rightChannel);
            limit(leftChannel);

            // Encode
            byte[] audioDataBack = new byte[len * 4];
            int pos = 0;
            for (int j = 0; j < len; j++) {
                short sampleRight = (short) rightChannel[j];
                short sampleLeft = (short) leftChannel[j];

                byte byte1 = (byte) (sampleRight & 0xFF);
                byte byte2 = (byte) ((sampleRight >> 8) & 0xFF);
                byte byte3 = (byte) (sampleLeft & 0xFF);
                byte byte4 = (byte) ((sampleLeft >> 8) & 0xFF);

                audioDataBack[pos] = byte1;
                audioDataBack[pos + 1] = byte2;
                audioDataBack[pos + 2] = byte3;
                audioDataBack[pos + 3] = byte4;
                pos += 4;
            }
            return audioDataBack;
        } else if (numChannels == 1) {
            this.rightChannel = buffer;

            // Apply effects
            for (int key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.onFinnish();
            }
            limit(rightChannel);

            // Encode
            byte[] audioDataBack = new byte[len * 2];
            int pos = 0;
            for (int j = 0; j < len; j++) {
                short sampleRight = (short) rightChannel[j];

                byte byte1 = (byte) (sampleRight & 0xFF);
                byte byte2 = (byte) ((sampleRight >> 8) & 0xFF);

                audioDataBack[pos] = byte1;
                audioDataBack[pos + 1] = byte2;
                pos += 2;
            }
            return audioDataBack;
        }
        return null;
    }

    private void limit(int[] channel) {
        for (int i = 0; i < channel.length; i++) {
            int c = channel[i];
            if (c < -32768) {
                c = -32768;
            } else if (c > 32767) {
                c = 32767;
            }
            channel[i] = c;
        }
    }

    public int[] getRightChannel() {
        return this.rightChannel;
    }

    public int[] getLeftChannel() {
        return this.leftChannel;
    }

    public static final float getDelay(AudioFormat format, int bufferSize) {
        float sampleRate = format.getSampleRate();
        float bitDepth = format.getSampleSizeInBits();
        float numChannels = format.getChannels();
        return 8.0f / (bitDepth * sampleRate * numChannels) * bufferSize;
    }
}
