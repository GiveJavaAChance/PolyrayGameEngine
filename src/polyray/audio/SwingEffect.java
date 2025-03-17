package polyray.audio;

public class SwingEffect implements Effect {

    private float vR, pR;
    private float vL, pL;
    private float damp, prevDamp;
    private float dt, prevDt;

    public SwingEffect(float damp, float dt) {
        this.damp = damp;
        this.prevDamp = damp;
        this.dt = dt;
        this.prevDt = dt;
    }

    public void setSwing(float damp, float dt) {
        this.damp = damp;
        this.dt = dt;
    }

    @Override
    public void computeRight(int[] channel) {
        float[] p = compute(channel, vR, pR);
        vR = p[0];
        pR = p[1];
    }

    @Override
    public void computeLeft(int[] channel) {
        float[] p = compute(channel, vL, pL);
        vL = p[0];
        pL = p[1];
    }

    @Override
    public void onStart() {
    }

    @Override
    public void onFinnish() {
        this.prevDamp = this.damp;
        this.prevDt = this.dt;
    }

    private float[] compute(int[] channel, float v, float p) {
        float deltaDamp = prevDamp - damp;
        float deltaDt = dt - prevDt;
        for (int i = 0; i < channel.length; i++) {
            float t = (float) i / channel.length;
            float d = prevDt + t * deltaDt;
            float da = prevDamp + t * deltaDamp;
            float force = (channel[i] - p);
            v += force * d;
            v *= da;
            p += v * d;
            if (p < -32768.0f) {
                p = -32768.0f;
            } else if (p > 32767.0f) {
                p = 32767.0f;
            }
            channel[i] = (int) p;
        }
        return new float[]{v, p};
    }
}
