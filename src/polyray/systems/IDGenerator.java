package polyray.systems;

import java.util.BitSet;
import java.util.Iterator;

public class IDGenerator {

    private final BitSet set;
    private int count = 0;

    public IDGenerator() {
        this.set = new BitSet();
    }

    public final int getNewID() {
        int id = set.nextClearBit(0);
        set.set(id);
        count++;
        return id;
    }

    public final void freeID(int ID) {
        if (ID < 0 || !set.get(ID)) {
            return;
        }
        set.clear(ID);
        count--;
    }
    
    public final void clear() {
        set.clear();
        count = 0;
    }

    public final boolean contains(int ID) {
        if (ID < 0) {
            return false;
        }
        return set.get(ID);
    }
    
    public final int size() {
        return count;
    }

    public final Iterator<Integer> getIDs() {
        return set.stream().iterator();
    }
}
