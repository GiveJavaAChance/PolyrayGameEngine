package polyray.systems.loader;

import java.io.IOException;
import polyray.systems.item.ItemInfo;
import polyray.systems.item.ItemRegistry;
import polyray.systems.item.P2IN;
import polyray.systems.item.Property;
import polyray.systems.item.PropertyRegistry;

public class ItemLoader {

    public static final void loadDirect(ClassLoader loader, String name) {
        try {
            P2IN p = new P2IN(new String(loader.getResourceAsStream(name).readAllBytes()));
            ItemLoader.load(p, loader);
        } catch (IOException e) {
        }
    }

    private static void load(P2IN p, ClassLoader loader) {
        String name = p.getName();
        float weight = Float.parseFloat(p.getContent("Weight").getContents());
        String textureName = p.getContent("TextureBase").getContents();
        int textureID = ItemTextureLoader.loadTextures(textureName, loader) + Integer.parseInt(p.getContent("TextureOffset").getContents());
        P2IN[] stats = p.getContent("Stats").listChildren();
        String[] statNames = new String[stats.length];
        int[] statRanges = new int[stats.length];
        for (int i = 0; i < stats.length; i++) {
            P2IN stat = stats[i];
            statNames[i] = stat.getName();
            statRanges[i] = Integer.parseInt(stat.getContents());
        }
        String pStr = p.getContent("Properties").getContents();
        String[] properties = pStr.substring(1, pStr.length() - 1).split(",");
        Property[] props = new Property[properties.length];
        for (int i = 0; i < properties.length; i++) {
            props[i] = PropertyRegistry.create(properties[i].trim());
        }
        ItemRegistry.register(new ItemInfo(name, weight, textureID, statNames, statRanges, props));
    }
}
