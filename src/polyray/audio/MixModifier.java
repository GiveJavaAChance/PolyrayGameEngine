package polyray.audio;

public class MixModifier implements Effect {

    private final Effect a, b;
    private final int[] copy;
    private float mix, prevMix;

    public MixModifier(Effect a, Effect b, float mix, int bufferSize) {
        this.a = a;
        this.b = b;
        this.mix = mix;
        this.prevMix = mix;
        this.copy = new int[bufferSize];
    }

    public void setMix(float mix) {
        this.mix = mix;
    }

    @Override
    public void computeRight(int[] channel) {
        System.arraycopy(channel, 0, copy, 0, channel.length);
        a.computeRight(channel);
        b.computeRight(copy);
        float deltaMix = mix - prevMix;
        for (int i = 0; i < channel.length; i++) {
            float m = prevMix + deltaMix * (float) i / channel.length;
            channel[i] += (int) (m * (copy[i] - channel[i]));
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        System.arraycopy(channel, 0, copy, 0, channel.length);
        a.computeLeft(channel);
        b.computeLeft(copy);
        float deltaMix = mix - prevMix;
        for (int i = 0; i < channel.length; i++) {
            float m = prevMix + deltaMix * (float) i / channel.length;
            channel[i] += (int) (m * (copy[i] - channel[i]));
        }
    }

    @Override
    public void onStart() {
        a.onStart();
        b.onStart();
    }

    @Override
    public void onFinnish() {
        a.onFinnish();
        b.onFinnish();
        this.prevMix = this.mix;
    }

}
