package polyray.audio;

public class DelayModifier implements Effect {

    private final Effect e;
    private final int[] delayRight, delayLeft;

    public DelayModifier(Effect e, int delay, int bufferSize) {
        this.e = e;
        this.delayLeft = new int[delay + bufferSize];
        this.delayRight = new int[delay + bufferSize];
    }

    @Override
    public void computeRight(int[] channel) {
        e.computeRight(channel);
        System.arraycopy(channel, 0, delayRight, delayRight.length - channel.length, channel.length);
        System.arraycopy(delayRight, 0, channel, 0, channel.length);
        System.arraycopy(delayRight, channel.length, delayRight, 0, delayRight.length - channel.length);
    }

    @Override
    public void computeLeft(int[] channel) {
        e.computeLeft(channel);
        System.arraycopy(channel, 0, delayLeft, delayLeft.length - channel.length, channel.length);
        System.arraycopy(delayLeft, 0, channel, 0, channel.length);
        System.arraycopy(delayLeft, channel.length, delayLeft, 0, delayLeft.length - channel.length);
    }

    @Override
    public void onStart() {
        e.onStart();
    }

    @Override
    public void onFinnish() {
        e.onFinnish();
    }

}
