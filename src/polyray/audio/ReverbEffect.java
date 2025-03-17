package polyray.audio;

public class ReverbEffect implements Effect {

    private final int[] reverbRight;
    private final int[] reverbLeft;
    private final float reverb;
    private final float mix;

    public ReverbEffect(int size, float reverb, float mix) {
        this.reverbRight = new int[size];
        this.reverbLeft = new int[size];
        this.reverb = reverb;
        this.mix = mix;
    }

    @Override
    public void computeRight(int[] channel) {
        compute(channel, reverbRight);
    }

    @Override
    public void computeLeft(int[] channel) {
        compute(channel, reverbLeft);
    }

    private void compute(int[] channel, int[] reverbChannel) {
        for (int i = 0; i < channel.length; i++) {
            int rev = reverbChannel[i];
            int cha = channel[i];
            int r = (int) (cha + reverb * (rev - cha));
            reverbChannel[i] = r;
            channel[i] += (r - cha) * mix;
        }
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onFinnish() {
    }

}
