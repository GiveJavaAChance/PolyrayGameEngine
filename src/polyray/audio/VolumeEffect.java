package polyray.audio;

public class VolumeEffect implements Effect {

    private float volume, prevVolume;

    public VolumeEffect(float volume) {
        this.volume = volume;
        this.prevVolume = volume;
    }

    public void setVolume(float volume) {
        this.volume = volume;
    }

    @Override
    public void computeRight(int[] channel) {
        compute(channel);
    }

    @Override
    public void computeLeft(int[] channel) {
        compute(channel);
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onFinnish() {
        this.prevVolume = this.volume;
    }

    private void compute(int[] channel) {
        float delta = volume - prevVolume;
        for (int i = 0; i < channel.length; i++) {
            float t = (float) i / channel.length;
            float vol = prevVolume + t * delta;
            channel[i] *= vol;
        }
    }

}
