package polyray.audio;

public class LoFiEffect implements Effect {

    private int lofi, prevLofi;
    private float sens, prevSens;
    private int counterR = 0, counterL = 0;
    private int cR = 0, cL = 0;

    public LoFiEffect(int lofi, float sens) {
        this.lofi = lofi;
        this.prevLofi = lofi;
        this.sens = sens;
        this.prevSens = sens;
    }

    public void setLoFi(int lofi, float sens) {
        this.lofi = lofi;
        this.sens = sens;
    }

    @Override
    public void computeRight(int[] channel) {
        int[] p = compute(channel, counterR, cR);
        counterR = p[0];
        cR = p[1];
    }

    @Override
    public void computeLeft(int[] channel) {
        int[] p = compute(channel, counterL, cL);
        counterL = p[0];
        cL = p[1];
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onFinnish() {
        this.prevLofi = this.lofi;
        this.prevSens = this.sens;
    }

    private int[] compute(int[] channel, int counter, int c) {
        int deltaLofi = lofi - prevLofi;
        int startSens = (int) ((1.0f - prevSens) * 32767.0f);
        int deltaSens = (int) ((prevSens - sens) * 32767.0f);
        for (int i = 0; i < channel.length; i++) {
            float t = (float) i / channel.length;
            int l = (int) (prevLofi + t * deltaLofi);
            int s = (int) (startSens + t * deltaSens);
            if (counter >= l || Math.abs(c - channel[i]) > s) {
                c = channel[i];
                counter = 0;
            }
            counter++;
            channel[i] = c;
        }
        return new int[]{counter, c};
    }

}
