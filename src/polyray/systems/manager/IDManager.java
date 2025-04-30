package polyray.systems.manager;

import java.util.BitSet;

public class IDManager {

    private static final BitSet set = new BitSet();

    public static final int getNewID() {
        int id = set.nextClearBit(0);
        set.set(id);
        return id;
    }

    public static final void freeID(int ID) {
        if(ID < 0) {
            return;
        }
        set.clear(ID);
    }

    public static final long getGlobalID(int ID) {
        return (((long) ServerManager.getClientID()) & 0xFFFFFFFFl) << 32l | ((long) (ID & 0xFFFFFFFFl));
    }
}
