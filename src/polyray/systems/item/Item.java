package polyray.systems.item;

import polyray.systems.entity.Entity;

public class Item {

    public static final int STACK_SIZE = 120;
    
    public int count;
    public final ItemInfo info;
    public final int[] stats;

    public Item(int count, ItemInfo info) {
        this.count = count;
        this.info = info;
        this.stats = new int[info.statNames.length];
    }
    
    public int add(int count) {
        this.count += count;
        if(this.count > STACK_SIZE) {
            int rem = this.count - STACK_SIZE;
            this.count = STACK_SIZE;
            return rem;
        }
        return 0;
    }

    public int getStat(String name) {
        Integer stat = info.statLookup.get(name);
        if (stat == null) {
            throw new IllegalArgumentException("Unknown Stat: " + name);
        }
        return stats[stat];
    }

    public void setStat(String name, int value) {
        Integer stat = info.statLookup.get(name);
        if (stat == null) {
            throw new IllegalArgumentException("Unknown Stat: " + name);
        }
        int max = info.statRanges[stat];
        if (value > max) {
            value = max;
        }
        stats[stat] = value;
    }

    public void changeStat(String name, int amt) {
        Integer stat = info.statLookup.get(name);
        if (stat == null) {
            throw new IllegalArgumentException("Unknown Stat: " + name);
        }
        int value = stats[stat] + amt;
        int max = info.statRanges[stat];
        if (value < 0) {
            value = 0;
        } else if (value > max) {
            value = max;
        }
        stats[stat] = value;
    }

    public void use(Entity e) {
        if (this.count <= 0) {
            return;
        }
        for (Property p : info.properties) {
            p.use(this, e);
        }
    }

    public void swing(Entity e) {
        if (this.count <= 0) {
            return;
        }
        for (Property p : info.properties) {
            p.swing(this, e);
        }
    }
}
