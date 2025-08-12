package polyray.systems;

import java.util.ArrayList;
import java.util.function.DoubleConsumer;

public class FixedUpdate {

    private final ArrayList<DoubleConsumer> updates;
    private double fixedDt;
    private double timeoutThreshold;
    private double timeRemaining = 0.0d;

    public FixedUpdate(double fixedDt, double timeoutThreshold) {
        this.updates = new ArrayList<>();
        this.fixedDt = fixedDt;
        this.timeoutThreshold = timeoutThreshold;
    }

    public final void setFixedDt(double fixedDt) {
        this.fixedDt = fixedDt;
    }

    public final void setTimeoutThreshold(double timeoutThreshold) {
        this.timeoutThreshold = timeoutThreshold;
    }

    public final void addUpdate(DoubleConsumer update) {
        updates.add(update);
    }

    public final void update(double dt) {
        timeRemaining += dt;
        long startTime = System.nanoTime();
        while (timeRemaining >= fixedDt) {
            timeRemaining -= fixedDt;
            for (DoubleConsumer update : updates) {
                update.accept(fixedDt);
            }
            if((System.nanoTime() - startTime) / 1000000000.0d > timeoutThreshold) {
                timeRemaining = 0.0d;
                return;
            }
        }
    }
}
