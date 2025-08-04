package polyray;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ShaderPreprocessor {

    private static final DecimalFormat df = new DecimalFormat("0.######");

    private static final Pattern APPEND_PATTERN = Pattern.compile("#append\\s+\"([^\"]+)\";");
    private static final Pattern SIGNATURE_PATTERN = Pattern.compile("(#override\\s+)?([a-zA-Z][a-zA-Z0-9_]*)\\s+([a-zA-Z][a-zA-Z0-9_]*)\\s*\\(([^)]*)\\)\\s*\\{", Pattern.DOTALL);

    private final String[] shaderCodes;
    private final boolean isCompute;

    static {
        df.setDecimalFormatSymbols(DecimalFormatSymbols.getInstance(Locale.US));
    }

    private ShaderPreprocessor(boolean isCompute, String... shaderCodes) {
        this.shaderCodes = shaderCodes;
        this.isCompute = isCompute;
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
                String fileContent = "";
                try {
                    fileContent = readFile(fileName);
                } catch (Exception e) {
                    try {
                        fileContent = readLocalFile(fileName);
                    } catch (Exception ex) {
                        System.err.println("Could not find file in shader append: " + fileName);
                    }
                }
                matcher.appendReplacement(buffer, Matcher.quoteReplacement(fileContent));
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

    public ShaderProgram createProgram(String name, int index) {
        if (isCompute) {
            return ShaderProgram.fromSource(shaderCodes[0], name, index);
        }
        return ShaderProgram.fromSource(shaderCodes[0], shaderCodes[1], name, index);
    }

    public String[] getCodes() {
        return this.shaderCodes;
    }

    public static ShaderPreprocessor fromFiles(String vertexFile, String fragmentFile) {
        return new ShaderPreprocessor(false, readFile(vertexFile), readFile(fragmentFile));
    }

    public static ShaderPreprocessor fromFiles(String computeFile) {
        return new ShaderPreprocessor(true, readFile(computeFile));
    }

    public static ShaderPreprocessor fromSource(String vertexSource, String fragmentSource) {
        return new ShaderPreprocessor(false, vertexSource, fragmentSource);
    }

    public static ShaderPreprocessor fromSource(String computeSource) {
        return new ShaderPreprocessor(true, computeSource);
    }

    public static ShaderPreprocessor fromLocalFiles(String vertexFile, String fragmentFile) {
        return new ShaderPreprocessor(false, readLocalFile(vertexFile), readLocalFile(fragmentFile));
    }

    public static ShaderPreprocessor fromLocalFiles(String computeFile) {
        return new ShaderPreprocessor(true, readLocalFile(computeFile));
    }

    private static String readFile(String filePath) {
        StringBuilder str = new StringBuilder();
        try ( BufferedReader br = new BufferedReader(new InputStreamReader(ResourceLoader.getLoader().getResourceAsStream(filePath)))) {
            String line;
            while ((line = br.readLine()) != null) {
                str.append(line);
                str.append("\n");
            }
        } catch (IOException e) {
        }
        return str.toString();
    }

    private static String readLocalFile(String filePath) {
        StringBuilder str = new StringBuilder();
        try ( BufferedReader br = new BufferedReader(new InputStreamReader(ShaderProgram.class.getResourceAsStream("shaders/" + filePath)))) {
            String line;
            while ((line = br.readLine()) != null) {
                str.append(line);
                str.append("\n");
            }
        } catch (IOException e) {
        }
        return str.toString();
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
