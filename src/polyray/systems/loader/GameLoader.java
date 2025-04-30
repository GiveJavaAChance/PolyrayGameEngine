package polyray.systems.loader;

import java.util.ArrayList;
import java.util.function.Supplier;
import polyray.systems.item.Property;
import polyray.systems.item.PropertyRegistry;

public class GameLoader {

    private static final ArrayList<String> items = new ArrayList<>();
    private static final ArrayList<PropertyRegistration> props = new ArrayList<>();

    public static final void addProperty(String name, Supplier<Property> s) {
        props.add(new PropertyRegistration(name, s));
    }

    public static final void addItemFile(String file) {
        items.add(file);
    }

    public static final void load(ClassLoader loader) {
        for (PropertyRegistration p : props) {
            PropertyRegistry.register(p.name, p.s);
        }
        for (String item : items) {
            ItemLoader.loadDirect(loader, item);
        }
        ModLoader.load();
    }

    private static class PropertyRegistration {

        final String name;
        final Supplier<Property> s;

        public PropertyRegistration(String name, Supplier<Property> s) {
            this.name = name;
            this.s = s;
        }
    }

}
