package polyray.audio;

public interface Effect {
    public void computeRight(int[] channel);
    public void computeLeft(int[] channel);
    public void onStart();
    public void onFinnish();
}
