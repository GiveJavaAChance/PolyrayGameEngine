package polyray.audio;

public class MuffleEffect implements Effect {

    private final SwingEffect swing;

    public MuffleEffect(float muffle) {
        this.swing = new SwingEffect(0.0f, 0.0f);
        setMuffle(muffle);
    }

    public final void setMuffle(float t) {
        t = Math.max(Math.min(t, 1.0f) * 2.0f, 0.0f);
        float dt0 = 1.599f;
        float d0 = 0.655f;
        float dt1 = 0.95f;
        float d1 = 0.63f;
        float dt2 = 0.015f;
        float d2 = 0.016f;
        float d = t <= 1.0f ? (d0 + t * (d1 - d0)) : (d1 + (t - 1.0f) * (d2 - d1));
        float dt = t <= 1.0f ? (dt0 + t * (dt1 - dt0)) : (dt1 + (t - 1.0f) * (dt2 - dt1));
        this.swing.setSwing(d, dt);
    }

    @Override
    public void computeRight(int[] channel) {
        swing.computeRight(channel);
    }

    @Override
    public void computeLeft(int[] channel) {
        swing.computeLeft(channel);
    }

    @Override
    public void onStart() {
        swing.onStart();
    }

    @Override
    public void onFinnish() {
        swing.onFinnish();
    }

}
