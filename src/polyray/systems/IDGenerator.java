package polyray.systems;

import java.util.BitSet;

public class IDGenerator {

    private final BitSet set;
    
    public IDGenerator() {
        this.set = new BitSet();
    }

    public final int getNewID() {
        int id = set.nextClearBit(0);
        set.set(id);
        return id;
    }

    public final void freeID(int ID) {
        if (ID < 0) {
            return;
        }
        set.clear(ID);
    }
}
