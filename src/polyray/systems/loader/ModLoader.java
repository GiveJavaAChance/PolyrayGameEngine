package polyray.systems.loader;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.ArrayList;
import java.util.Enumeration;
import java.util.jar.JarEntry;
import java.util.jar.JarFile;
import polyray.systems.item.Property;
import polyray.systems.item.PropertyRegistry;

public class ModLoader {

    public static void load() {
        try {
            File jarDir = new File(ModLoader.class.getProtectionDomain().getCodeSource().getLocation().toURI()).getParentFile();
            File modsDir = new File(jarDir, "mods");
            if (!modsDir.exists() || !modsDir.isDirectory()) {
                System.out.println("No mods directory found. Skipping mod loading.");
                return;
            }
            File[] modJars = modsDir.listFiles((dir, name) -> name.endsWith(".jar"));
            if (modJars == null || modJars.length == 0) {
                System.out.println("No mod jars found.");
                return;
            }
            for (File modJar : modJars) {
                loadMod(modJar);
            }
        } catch (URISyntaxException e) {
        }
    }

    private static void loadMod(File jarFile) {
        try {
            String modName = jarFile.getName();
            modName = modName.substring(0, modName.length() - 4);
            URLClassLoader loader = new URLClassLoader(new URL[]{jarFile.toURI().toURL()}, ModLoader.class.getClassLoader());
            ArrayList<String> itemFiles = new ArrayList<>();
            try ( JarFile jar = new JarFile(jarFile)) {
                Enumeration<JarEntry> entries = jar.entries();
                while (entries.hasMoreElements()) {
                    JarEntry entry = entries.nextElement();
                    String entryName = entry.getName();

                    if (entryName.endsWith(".class")) {
                        loadClass(entryName, loader, modName);
                    } else if (entryName.endsWith(".p2in")) {
                        itemFiles.add(entryName);
                    }
                }
            }
            for (String name : itemFiles) {
                ItemLoader.loadDirect(loader, name);
            }
        } catch (IOException e) {
            System.err.println("Failed to load mod jar: " + jarFile.getName());
        }
    }

    private static void loadClass(String entryName, URLClassLoader loader, String modName) {
        String className = entryName
                .replace('/', '.')
                .substring(0, entryName.length() - 6);
        try {
            Class<?> clazz = Class.forName(className, false, loader);
            String name = clazz.getSimpleName();
            if (isValidProperty(clazz, name, modName)) {
                Constructor<?> c = clazz.getDeclaredConstructor();
                PropertyRegistry.register(name, () -> {
                    try {
                        return (Property) c.newInstance();
                    } catch (IllegalAccessException | IllegalArgumentException | InstantiationException | SecurityException | InvocationTargetException e) {
                        throw new RuntimeException(e);
                    }
                });
                System.out.println("Registered property: " + name + " from mod " + modName);
            }
        } catch (ClassNotFoundException | NoClassDefFoundError | NoSuchMethodException | SecurityException e) {
            System.err.println("Failed to load class: " + className + " from " + modName + " (Maybe missing dependency?)");
        }
    }

    private static boolean isValidProperty(Class<?> clazz, String name, String modName) {
        if (!Property.class.isAssignableFrom(clazz)) {
            return false;
        }
        for (Constructor<?> constructor : clazz.getDeclaredConstructors()) {
            if (constructor.getParameterCount() == 0) {
                return true;
            }
        }
        System.err.println("The item property " + name + " from the mod " + modName + " doesn't have a default constructor.");
        return false;
    }
}
