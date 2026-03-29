package polyray.systems.bodfile;

import java.util.HashMap;

public class ObjectMap<A> extends HashMap<A, Object> {

    @SuppressWarnings("unchecked")
    public <C> C getAs(A key) {
        return (C) get(key);
    }
}
