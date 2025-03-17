package polyray.audio;

public class MultiplyModifier implements Effect {

    private final Effect a, b;
    private final int[] copy;

    public MultiplyModifier(Effect a, Effect b, int bufferSize) {
        this.a = a;
        this.b = b;
        this.copy = new int[bufferSize];
    }

    @Override
    public void computeRight(int[] channel) {
        System.arraycopy(channel, 0, copy, 0, channel.length);
        a.computeRight(channel);
        b.computeRight(copy);
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) ((((long) channel[i]) * ((long) copy[i])) >> 15l);
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        System.arraycopy(channel, 0, copy, 0, channel.length);
        a.computeLeft(channel);
        b.computeLeft(copy);
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) ((((long) channel[i]) * ((long) copy[i])) >> 15l);
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
    }

}
