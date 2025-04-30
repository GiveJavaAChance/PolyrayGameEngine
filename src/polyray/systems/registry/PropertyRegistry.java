package polyray.systems.item;

import java.util.HashMap;
import java.util.function.Supplier;

public class PropertyRegistry {

    private static final HashMap<String, Supplier<? extends Property>> properties = new HashMap<>();

    public static final void register(String name, Supplier<? extends Property> p) {
        System.out.println("Property registered: " + name);
        properties.put(name, p);
    }

    public static final Property create(String name) {
        return properties.get(name).get();
    }
}
