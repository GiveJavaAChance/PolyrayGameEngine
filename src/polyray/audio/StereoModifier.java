package polyray.audio;

public class StereoModifier implements Effect {

    private final Effect right, left;
    
    public StereoModifier(Effect right, Effect left) {
        this.right = right;
        this.left = left;
    }
    
    @Override
    public void computeRight(int[] channel) {
        right.computeRight(channel);
    }

    @Override
    public void computeLeft(int[] channel) {
        left.computeRight(channel);
    }

    @Override
    public void onStart() {
        right.onStart();
        left.onStart();
    }

    @Override
    public void onFinnish() {
        right.onFinnish();
        left.onFinnish();
    }
    
}
