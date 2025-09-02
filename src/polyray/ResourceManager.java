package polyray;

import java.awt.Graphics2D;
import java.awt.image.BufferedImage;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.function.Function;
import javax.imageio.ImageIO;

public class ResourceManager {

    private static final ArrayList<Loader> resourceLoaders = new ArrayList<>();

    public static void addResource(Class<?> clazz) {
        resourceLoaders.add(new Loader(clazz, null));
    }

    public static void addResource(Class<?> clazz, String dir) {
        resourceLoaders.add(new Loader(clazz, dir));
    }

    public static void addResource(File file) {
        resourceLoaders.add(new Loader(file));
    }

    public static InputStream getResource(String name) {
        for (int i = resourceLoaders.size() - 1; i >= 0; i--) {
            InputStream in = resourceLoaders.get(i).load(name);
            if (in != null) {
                return in;
            }
        }
        return ResourceManager.class.getResourceAsStream("shaders/" + name);
    }

    public static InputStreamReader getResourceAsReader(String name) {
        InputStream in = getResource(name);
        return in == null ? null : new InputStreamReader(in);
    }

    public static String getResourceAsString(String name) {
        try ( InputStream in = getResource(name)) {
            return in == null ? null : new String(in.readAllBytes());
        } catch (IOException e) {
            return null;
        }
    }

    public static BufferedImage getResourceAsImage(String name) {
        try ( InputStream in = getResource(name)) {
            return in == null ? null : ImageIO.read(in);
        } catch (IOException e) {
            return null;
        }
    }

    public static BufferedImage getResourceAsImage(String name, int type) {
        BufferedImage tmp = getResourceAsImage(name);
        if (tmp == null || tmp.getType() == type) {
            return tmp;
        }
        BufferedImage img = new BufferedImage(tmp.getWidth(), tmp.getHeight(), type);
        Graphics2D g = img.createGraphics();
        g.drawImage(tmp, 0, 0, null);
        g.dispose();
        tmp.flush();
        return img;
    }

    private static class Loader {

        private final Function<String, InputStream> loader;

        public Loader(Class<?> clazz, String dir) {
            loader = name -> clazz.getResourceAsStream(dir == null ? name : (dir + "/" + name));
        }

        public Loader(File file) {
            loader = name -> {
                File f = new File(file, name);
                try {
                    return f.exists() ? new FileInputStream(f) : null;
                } catch (IOException e) {
                    return null;
                }
            };
        }

        public final InputStream load(String name) {
            return loader.apply(name);
        }
    }
}
