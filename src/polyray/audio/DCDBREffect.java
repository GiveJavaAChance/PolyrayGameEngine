package polyray.audio;

public class DCDBREffect implements Effect {

    private final float[] collectorRight, collectorLeft;

    private final int samplesPerBuffer;
    private final float[] volumesRight, volumesLeft;
    private final int[] delaysRight, delaysLeft;

    public DCDBREffect(int samplesPerBuffer, int maxDelay, int bufferSize) {
        this(samplesPerBuffer, maxDelay, bufferSize, 2.0d);
    }

    public DCDBREffect(int samplesPerBuffer, int maxDelay, int bufferSize, double power) {
        this.collectorRight = new float[maxDelay + bufferSize];
        this.collectorLeft = new float[maxDelay + bufferSize];
        this.samplesPerBuffer = samplesPerBuffer;
        volumesRight = new float[samplesPerBuffer];
        delaysRight = new int[samplesPerBuffer];
        volumesLeft = new float[samplesPerBuffer];
        delaysLeft = new int[samplesPerBuffer];
        for (int i = 0; i < samplesPerBuffer; i++) {
            double rr = Math.pow(Math.random(), power);
            double rl = Math.pow(Math.random(), power);
            volumesRight[i] = (float) (1.0d - rr);
            delaysRight[i] = (int) (rr * maxDelay);
            volumesLeft[i] = (float) (1.0d - rl);
            delaysLeft[i] = (int) (rl * maxDelay);
        }
    }

    @Override
    public void computeRight(int[] channel) {
        for (int i = 0; i < samplesPerBuffer; i++) {
            int idx = delaysRight[i];
            float vol = volumesRight[i];
            for (int j = 0; j < channel.length; j++) {
                collectorRight[j + idx] += channel[j] * vol;
            }
        }
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) collectorRight[i];
        }
        System.arraycopy(collectorRight, channel.length, collectorRight, 0, collectorRight.length - channel.length);
        for (int i = collectorRight.length - channel.length; i < collectorRight.length; i++) {
            collectorRight[i] = 0.0f;
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        for (int i = 0; i < samplesPerBuffer; i++) {
            int idx = delaysLeft[i];
            float vol = volumesLeft[i];
            for (int j = 0; j < channel.length; j++) {
                collectorLeft[j + idx] += channel[j] * vol;
            }
        }
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) collectorLeft[i];
        }
        System.arraycopy(collectorLeft, channel.length, collectorLeft, 0, collectorLeft.length - channel.length);
        for (int i = collectorLeft.length - channel.length; i < collectorLeft.length; i++) {
            collectorLeft[i] = 0.0f;
        }
    }

    @Override
    public void onStart() {

    }

    @Override
    public void onFinnish() {

    }
}
