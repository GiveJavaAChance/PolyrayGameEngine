package polyray.audio;

public class PanEffect implements Effect {

    private float pan, prevPan;

    public PanEffect(float pan) {
        this.pan = pan;
        this.prevPan = pan;
    }

    public void setPan(float pan) {
        this.pan = pan;
    }

    @Override
    public void computeRight(int[] channel) {
        float delta = pan - prevPan;
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) (channel[i] * Math.sin((prevPan + (double) i / channel.length * delta + 1.0d) * Math.PI / 4));
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        float delta = pan - prevPan;
        for (int i = 0; i < channel.length; i++) {
            channel[i] = (int) (channel[i] * Math.cos((prevPan + (double) i / channel.length * delta + 1.0d) * Math.PI / 4));
        }
    }

    @Override
    public void onStart() {

    }

    @Override
    public void onFinnish() {
        this.prevPan = this.pan;
    }

}
