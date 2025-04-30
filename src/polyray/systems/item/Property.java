package polyray.systems.item;

import polyray.systems.entity.Entity;

public abstract class Property {

    public abstract void use(Item item, Entity e);

    public abstract void swing(Item item, Entity e);
}
