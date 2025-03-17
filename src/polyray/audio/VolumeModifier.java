package polyray.audio;

public class VolumeModifier implements Effect {

    private final Effect e;
    private float volume, prevVolume;

    public VolumeModifier(Effect e, float volume) {
        this.e = e;
        this.volume = volume;
        this.prevVolume = volume;
    }

    public void setVolume(float volume) {
        this.volume = volume;
    }

    @Override
    public void computeRight(int[] channel) {
        e.computeRight(channel);
        float deltaVolume = volume - prevVolume;
        for (int i = 0; i < channel.length; i++) {
            channel[i] *= prevVolume + deltaVolume * (float) i / channel.length;
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        e.computeLeft(channel);
        float deltaVolume = volume - prevVolume;
        for (int i = 0; i < channel.length; i++) {
            channel[i] *= prevVolume + deltaVolume * (float) i / channel.length;
        }
    }

    @Override
    public void onStart() {
        e.onStart();
    }

    @Override
    public void onFinnish() {
        e.onFinnish();
        this.prevVolume = volume;
    }

}
