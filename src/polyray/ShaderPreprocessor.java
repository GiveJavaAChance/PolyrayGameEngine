package polyray;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Locale;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class ShaderPreprocessor {

    private static final DecimalFormat df = new DecimalFormat("0.######");

    private static final Pattern BINDING_PATTERN = Pattern.compile("(layout\\s*\\([^)]*binding\\s*=\\s*(\\d+)\\s*,\\s*[^)]*\\)\\s*[^;]*;)");
    private static final Pattern APPEND_PATTERN = Pattern.compile("#append\\s+\"([^\"]+)\";");

    private final String[] shaderCodes;
    private final boolean isCompute;

    static {
        df.setDecimalFormatSymbols(DecimalFormatSymbols.getInstance(Locale.US));
    }

    private ShaderPreprocessor(boolean isCompute, String... shaderCodes) {
        this.shaderCodes = shaderCodes;
        this.isCompute = isCompute;
    }

    public void setBindingPoint(String name, int binding) {
        for (int i = 0; i < shaderCodes.length; i++) {
            String shaderCode = shaderCodes[i];
            // Create a matcher for the shader code
            Matcher matcher = BINDING_PATTERN.matcher(shaderCode);

            while (matcher.find()) {
                String layoutBlock = matcher.group(1);  // The entire layout block
                // Check if the texture/buffer name is part of this layout block
                if (layoutBlock.contains(name)) {
                    // Replace the binding number for this layout block
                    String updatedLayoutBlock = layoutBlock.replaceAll("binding\\s*=\\s*\\d+", "binding = " + binding);
                    // Replace the original block with the updated one
                    shaderCode = shaderCode.replace(layoutBlock, updatedLayoutBlock);
                    break;
                }
            }
            shaderCodes[i] = shaderCode;
        }
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

    static ShaderPreprocessor fromLocalFiles(String vertexFile, String fragmentFile) {
        return new ShaderPreprocessor(false, readLocalFile(vertexFile), readLocalFile(fragmentFile));
    }

    static ShaderPreprocessor fromLocalFiles(String computeFile) {
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
}
