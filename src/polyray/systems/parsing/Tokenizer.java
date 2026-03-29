package polyray.systems.parsing;

import java.util.ArrayList;

public class Tokenizer {

    private final TokenRegistry reg;
    private final char[] input;
    private final int length;
    private int pos = 0;

    public Tokenizer(TokenRegistry reg, String input) {
        this.reg = reg;
        this.input = input.toCharArray();
        this.length = input.length();
    }

    public ArrayList<Token> tokenize() {
        ArrayList<Token> tokens = new ArrayList<>();
        while (pos < length) {
            if (("" + input[pos]).matches("\\s")) {
                pos++;
                continue;
            }
            CharView w = new CharView(input, pos);
            int tokenID = reg.get(w);
            if (tokenID == -1) {
                throw new RuntimeException("Unexpected character: '" + input[pos] + "' at position " + pos);
            }
            tokens.add(new Token(new String(input, pos, w.len), tokenID));
            pos += w.len;
        }
        return tokens;
    }
}
