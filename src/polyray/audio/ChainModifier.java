
package polyray.audio;

public class ChainModifier implements Effect {

    private final Effect[] e;
    
    public ChainModifier(Effect... e) {
        this.e = e;
    }
    
    @Override
    public void computeRight(int[] channel) {
        for (Effect effect : e) {
            effect.computeRight(channel);
        }
    }

    @Override
    public void computeLeft(int[] channel) {
        for (Effect effect : e) {
            effect.computeLeft(channel);
        }
    }

    @Override
    public void onStart() {
        for (Effect effect : e) {
            effect.onStart();
        }
    }

    @Override
    public void onFinnish() {
        for (Effect effect : e) {
            effect.onFinnish();
        }
    }
    
}
