package polyray;

public class ResourceLoader {

    private static Class<?> loaderClazz = ShaderProgram.class;

    public static final void setResourceClass(Class<?> clazz) {
        if(clazz == null) {
            return;
        }
        loaderClazz = clazz;
    }

    public static Class<?> getLoader() {
        return loaderClazz;
    }
}
