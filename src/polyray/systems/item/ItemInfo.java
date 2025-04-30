package polyray.systems.item;

import java.util.HashMap;

public class ItemInfo {

    public final String name;
    public final float weight;
    public final int itemTextureID;
    public final Property[] properties;
    public final HashMap<String, Integer> statLookup;
    public final String[] statNames;
    public final int[] statRanges;

    public ItemInfo(String name, float weight, int itemTextureID, String[] statNames, int[] statRanges, Property[] properties) {
        this.name = name;
        this.weight = weight;
        this.itemTextureID = itemTextureID;
        this.properties = properties;
        this.statLookup = new HashMap<>();
        this.statNames = statNames;
        this.statRanges = statRanges;
        int idx = 0;
        for (String stat : statNames) {
            statLookup.put(stat, idx++);
        }
    }
}
