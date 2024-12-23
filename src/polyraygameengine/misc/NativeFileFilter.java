package polyraygameengine.misc;

public class NativeFileFilter {

    public static final NativeFileFilter ALL_FILES;
    public static final NativeFileFilter DIRECTORIES;
    
    protected String filter;
    
    static {
        ALL_FILES = new NativeFileFilter("All Files", "*");
        DIRECTORIES = new NativeFileFilter("Directories", "");
        DIRECTORIES.filter = "Directories|*.";
    }

    public NativeFileFilter(String displayName, String... extensions) {
        StringBuilder filterBuilder = new StringBuilder();
        filterBuilder.append(displayName);
        filterBuilder.append(" ");
        Object[] args = (Object[]) extensions;
        String name = String.format("(*.%s), ".repeat(extensions.length), args);
        filterBuilder.append(name.substring(0, name.length() - 2));
        filterBuilder.append("|");
        String format = String.format("*.%s;".repeat(extensions.length), args);
        filterBuilder.append(format.substring(0, format.length() - 1));
        this.filter = filterBuilder.toString();
    }

    public NativeFileFilter(String[] displayNames, String[] extensions) {
        if (displayNames.length != extensions.length) {
            throw new IllegalArgumentException("Display names and extensions arrays must have the same length.");
        }
        StringBuilder filterBuilder = new StringBuilder();
        int i = 0;
        for (String ext : extensions) {
            filterBuilder.append(formatFilter(displayNames[i], ext));
            if (i < extensions.length - 1) {
                filterBuilder.append("|");
            }
            i++;
        }
        this.filter = filterBuilder.toString();
    }

    private static String formatFilter(String name, String extension) {
        return String.format("%s (*.%s)|*.%s", name, extension, extension);
    }

    private static String formatExtension(String extension) {
        return "*." + extension;
    }
}
