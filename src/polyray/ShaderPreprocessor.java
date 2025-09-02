package polyray;

import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import static org.lwjgl.opengl.GL46.*;

public class ShaderPreprocessor {

    private static final DecimalFormat df = new DecimalFormat("0.######");

    private static final Pattern APPEND_PATTERN = Pattern.compile("#append\\s+\"([^\"]+)\";");
    private static final Pattern SIGNATURE_PATTERN = Pattern.compile("(#override\\s+)?([a-zA-Z][a-zA-Z0-9_]*)\\s+([a-zA-Z][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)\\s*\\{", Pattern.DOTALL);

    private final String[] shaderCodes;
    private final int[] shaderTypes;

    static {
        df.setDecimalFormatSymbols(DecimalFormatSymbols.getInstance(Locale.US));
    }

    private ShaderPreprocessor(String[] shaderCodes, int... shaderTypes) {
        this.shaderCodes = shaderCodes;
        this.shaderTypes = shaderTypes;
    }

    public void setInt(String name, int val) {
        String value = "" + val;
        for (int i = 0; i < shaderCodes.length; i++) {
            shaderCodes[i] = shaderCodes[i].replace(name, value);
        }
    }

    public void setFloat(String name, float val) {
        String value = df.format(val);
        for (int i = 0; i < shaderCodes.length; i++) {
            shaderCodes[i] = shaderCodes[i].replace(name, value);
        }
    }

    public void setDouble(String name, double val) {
        String value = df.format(val);
        for (int i = 0; i < shaderCodes.length; i++) {
            shaderCodes[i] = shaderCodes[i].replace(name, value);
        }
    }

    public void setString(String name, String value) {
        for (int i = 0; i < shaderCodes.length; i++) {
            shaderCodes[i] = shaderCodes[i].replace(name, value);
        }
    }

    public void appendAll() {
        for (int i = 0; i < shaderCodes.length; i++) {
            appendCode(i);
        }
    }

    private void appendCode(int index) {
        HashSet<String> appendedFiles = new HashSet<>();
        int appendCount = 1;
        while (appendCount != 0) {
            appendCount = 0;
            Matcher matcher = APPEND_PATTERN.matcher(shaderCodes[index]);
            StringBuffer buffer = new StringBuffer();

            while (matcher.find()) {
                String fileName = matcher.group(1);
                if (appendedFiles.contains(fileName)) {
                    matcher.appendReplacement(buffer, Matcher.quoteReplacement(""));
                    continue;
                }
                appendedFiles.add(fileName);
                String content = ResourceManager.getResourceAsString(fileName);
                if(content == null) {
                    System.err.println("Could not find file in shader append: " + fileName);
                    System.exit(1);
                }
                matcher.appendReplacement(buffer, Matcher.quoteReplacement(content));
                appendCount++;
            }
            matcher.appendTail(buffer);
            shaderCodes[index] = buffer.toString().replaceAll("\n{3,}", "\n\n");
        }
    }

    public void overrideAll() {
        for (int i = 0; i < shaderCodes.length; i++) {
            shaderCodes[i] = override(shaderCodes[i]);
        }
    }

    private String override(String code) {
        String b = code;
        Matcher matcher = SIGNATURE_PATTERN.matcher(code);
        ArrayList<FunctionSignature> functions = new ArrayList<>();
        while (matcher.find()) {
            boolean isOverride = matcher.group(1) != null;
            String returnType = matcher.group(2);
            String name = matcher.group(3);
            String paramStr = matcher.group(4);
            int start = matcher.end() - 1;
            int end = closingBracket(code, start);
            if (end == -1) {
                continue;
            }
            FunctionSignature sig = new FunctionSignature(code.substring(matcher.start(2), end + 1), returnType, name, paramStr.split(","));
            if (isOverride) {
                int idx = functions.lastIndexOf(sig);
                if (idx != -1) {
                    b = b.replaceAll("\\s+" + Pattern.quote("#override") + "\\s+" + Pattern.quote(sig.function) + "\\s*", "\n").replace(functions.get(idx).function, sig.function);
                    functions.set(idx, sig);
                }
            } else {
                functions.add(sig);
            }
        }
        return b;
    }

    public String[] getFunctionDefinitions() {
        String[] defines = new String[shaderCodes.length];
        for (int i = 0; i < shaderCodes.length; i++) {
            String code = shaderCodes[i];
            Matcher matcher = SIGNATURE_PATTERN.matcher(clean(code));
            StringBuilder builder = new StringBuilder();
            while (matcher.find()) {
                boolean isOverride = matcher.group(1) != null;
                String returnType = matcher.group(2);
                String name = matcher.group(3);
                String paramStr = matcher.group(4);
                int start = matcher.end() - 1;
                int end = closingBracket(code, start);
                if (end == -1) {
                    continue;
                }
                if (!isOverride) {
                    builder.append(new FunctionSignature(code.substring(matcher.start(2), end + 1), returnType, name, paramStr.split(",")).toString());
                    builder.append('\n');
                }
            }
            defines[i] = builder.toString();
        }
        return defines;
    }

    private int closingBracket(String code, int start) {
        int depth = 0;
        for (int i = start; i < code.length(); i++) {
            char c = code.charAt(i);
            if (c == '{') {
                depth++;
            } else if (c == '}') {
                depth--;
                if (depth == 0) {
                    return i;
                }
            }
        }
        return -1;
    }

    private String clean(String code) {
        return code.replaceAll("(?s)/\\*.*?\\*/", "").replaceAll("//.*", "");
    }

    public ShaderProgram createProgram() {
        return ShaderProgram.customFromSource(shaderCodes, shaderTypes);
    }

    public String[] getCodes() {
        return this.shaderCodes;
    }

    public static ShaderPreprocessor fromFiles(String vertexFile, String fragmentFile) {
        return fromSource(ResourceManager.getResourceAsString(vertexFile), ResourceManager.getResourceAsString(fragmentFile));
    }

    public static ShaderPreprocessor fromFiles(String computeFile) {
        return fromSource(ResourceManager.getResourceAsString(computeFile));
    }

    public static ShaderPreprocessor customFromFiles(String[] files, int[] types) {
        if (files.length != types.length) {
            throw new IllegalArgumentException("source and type count mismatch.");
        }
        String[] sources = new String[files.length];
        for (int i = 0; i < sources.length; i++) {
            sources[i] = ResourceManager.getResourceAsString(files[i]);
        }
        return new ShaderPreprocessor(sources, types);
    }

    public static ShaderPreprocessor fromSource(String vertexSource, String fragmentSource) {
        return new ShaderPreprocessor(new String[]{vertexSource, fragmentSource}, GL_VERTEX_SHADER, GL_FRAGMENT_SHADER);
    }

    public static ShaderPreprocessor fromSource(String computeSource) {
        return new ShaderPreprocessor(new String[]{computeSource}, GL_COMPUTE_SHADER);
    }

    public static ShaderPreprocessor customFromSource(String[] sources, int[] types) {
        if (sources.length != types.length) {
            throw new IllegalArgumentException("source and type count mismatch.");
        }
        return new ShaderPreprocessor(sources, types);
    }

    private static class FunctionSignature {

        public final String returnType;
        public final String name;
        public final String[] paramTypes;
        public final String function;

        public FunctionSignature(String function, String returnType, String name, String[] params) {
            this.function = function;
            this.returnType = returnType;
            this.name = name;
            this.paramTypes = new String[params.length];
            for (int i = 0; i < params.length; i++) {
                this.paramTypes[i] = params[i].trim().split("\\s+")[0];
            }
        }

        @Override
        public boolean equals(Object obj) {
            if (obj instanceof FunctionSignature sig) {
                if (sig.name.equals(this.name) && sig.returnType.equals(this.returnType) && sig.paramTypes.length == this.paramTypes.length) {
                    for (int i = 0; i < this.paramTypes.length; i++) {
                        if (!sig.paramTypes[i].equals(this.paramTypes[i])) {
                            return false;
                        }
                    }
                    return true;
                }
            }
            return false;
        }

        @Override
        public int hashCode() {
            int hash = 3;
            hash = 71 * hash + Objects.hashCode(this.returnType);
            hash = 71 * hash + Objects.hashCode(this.name);
            hash = 71 * hash + Arrays.deepHashCode(this.paramTypes);
            return hash;
        }

        @Override
        public String toString() {
            return returnType + " " + name + "(" + String.join(", ", paramTypes) + ")";
        }
    }
}
