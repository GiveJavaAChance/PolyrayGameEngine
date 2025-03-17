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
    
    // Calculate right channel (Whenever the audio is mono, the mixer only uses this method)
    @Override
    public void computeRight(int[] channel) {
        compute(channel);
    }

    // Calculate left channel
    @Override
    public void computeLeft(int[] channel) {
        compute(channel);
    }
    
    // Triggered when the mixer is about to start calculating this effect
    @Override
    public void onStart() {
    }
    
    // Triggered when the mixer has finnished calculating this effect
    @Override
    public void onFinnish() {
        this.prevVolume = this.volume;
    }
    
    private void compute(int[] channel) { // Shared method for calculating volume on both left and right channels
        float delta = volume - prevVolume;
        for(int i = 0; i < channel.length; i++) {
            float t = (float) i / channel.length;
            float vol = prevVolume +  t * delta;
            channel[i] *= vol;
        }
    }
    
}
