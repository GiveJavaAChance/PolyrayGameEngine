package polyray.audio;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import javax.sound.sampled.AudioFormat;

public class SoundEffector {

    private final HashMap<Integer, Effect> effects = new HashMap<>();
    private final ArrayList<Integer> keys = new ArrayList<>();
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

    public void nextBuffer(int[] right, int[] left) {
        for (Integer key : keys) {
            Effect effect = effects.get(key);
            effect.onStart();
            effect.computeRight(right);
            effect.computeLeft(left);
            effect.onFinnish();
        }
        limit(right);
        limit(left);
    }

    public byte[] nextBuffer(byte[] buffer, boolean swapStereo) {
        int len = (buffer.length / numBytes) >> 1;
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
            for (Integer key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.computeLeft(leftChannel);
                effect.onFinnish();
            }
            limit(rightChannel);
            limit(leftChannel);

            // Encode
            return encode(rightChannel, leftChannel);
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
            for (Integer key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.onFinnish();
            }
            limit(rightChannel);

            // Encode
            return encode(rightChannel);
        }
        return null;
    }

    public byte[] nextBuffer(int[] channel) {
        int len = channel.length;
        if (numChannels == 2) {
            this.rightChannel = channel;
            this.leftChannel = new int[len];
            System.arraycopy(channel, 0, this.leftChannel, 0, len);

            // Apply effects
            for (Integer key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.computeLeft(leftChannel);
                effect.onFinnish();
            }
            limit(rightChannel);
            limit(leftChannel);

            // Encode
            return encode(rightChannel, leftChannel);
        } else if (numChannels == 1) {
            this.rightChannel = channel;

            // Apply effects
            for (Integer key : keys) {
                Effect effect = effects.get(key);
                effect.onStart();
                effect.computeRight(rightChannel);
                effect.onFinnish();
            }
            limit(rightChannel);

            // Encode
            return encode(rightChannel);
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

    private static byte[] encode(int[] right, int[] left) {
        int pos = 0;
        byte[] buffer = new byte[right.length << 2];
        for (int j = 0; j < right.length; j++) {
            short sampleRight = (short) right[j];
            short sampleLeft = (short) left[j];

            buffer[pos++] = (byte) (sampleRight & 0xFF);
            buffer[pos++] = (byte) ((sampleRight >> 8) & 0xFF);
            buffer[pos++] = (byte) (sampleLeft & 0xFF);
            buffer[pos++] = (byte) ((sampleLeft >> 8) & 0xFF);
        }
        return buffer;
    }

    private static byte[] encode(int[] channel) {
        byte[] buffer = new byte[channel.length << 1];
        int pos = 0;
        for (int j = 0; j < channel.length; j++) {
            short sample = (short) channel[j];

            buffer[pos++] = (byte) (sample & 0xFF);
            buffer[pos++] = (byte) ((sample >> 8) & 0xFF);
        }
        return buffer;
    }

    public static final float getDelay(AudioFormat format, int bufferSize) {
        float sampleRate = format.getSampleRate();
        float bitDepth = format.getSampleSizeInBits();
        float numChannels = format.getChannels();
        return 8.0f / (bitDepth * sampleRate * numChannels) * bufferSize;
    }
}
