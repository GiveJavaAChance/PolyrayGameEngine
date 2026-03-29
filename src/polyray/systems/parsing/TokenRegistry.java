package polyray.systems.parsing;

import java.util.HashMap;
import java.util.HashSet;
import java.util.Map.Entry;

public class TokenRegistry {

    public static final TokenParser GENERIC_IDENTIFIER = w -> {
        int len = -1;
        if (Character.isLetter(w.peek()) || w.peek() == '_') {
            w.step();
            len = 1;
            while (w.hasNext() && (Character.isLetterOrDigit(w.peek()) || w.peek() == '_')) {
                w.step();
                len++;
            }
        }
        return len;
    };
    public static final TokenParser INT = w -> {
        int len = 0;
        char[] ranges = {'0', '9'};
        if (w.hasNext()) {
            char c = w.peek();
            if (c == '+' || c == '-') {
                w.step();
                len++;
            }
        }
        if (w.remaining() > 1) {
            String str = w.get(2).toLowerCase();
            if (str.equals("0x")) {
                w.step(2);
                len += 2;
                ranges = new char[]{'0', '9', 'a', 'f', 'A', 'F'};
            } else if (str.equals("0b")) {
                w.step(2);
                len += 2;
                ranges = new char[]{'0', '1'};
            }
        }
        while (w.hasNext() && w.nextRange(ranges)) {
            w.step();
            len++;
        }
        if (w.hasNext()) {
            if (w.peek() == 'u' || w.peek() == 'U') {
                w.step();
                len++;
            }
        }
        return len == 0 ? -1 : len;
    };
    public static final TokenParser FLOAT = w -> {
        int len = 0;
        char[] ranges = {'0', '9'};
        if (w.hasNext()) {
            char c = w.peek();
            if (c == '+' || c == '-') {
                w.step();
                len++;
            }
        }
        while (w.hasNext() && (w.nextRange(ranges) || w.peek() == '.')) {
            w.step();
            len++;
        }
        return len == 0 ? -1 : len;
    };
    public static final TokenParser STRING = w -> {
        if (w.peek() != '\"') {
            return -1;
        }
        int len = 1;
        boolean closed = false;
        int count = 0;
        while (w.hasNext()) {
            w.step();
            char c = w.peek();
            len++;
            if (c == '\"' && (count & 1) == 0) {
                closed = true;
                break;
            }
            if (c == '\\') {
                count++;
            } else {
                count = 0;
            }
        }
        if (!closed) {
            w.ex("Unterminated string literal");
        }
        return len;
    };
    public static final TokenParser UUID = w -> {
        int[] groups = {8, 4, 4, 4, 12};
        int len = 0;
        for (int g = 0; g < groups.length; g++) {
            for (int i = 0; i < groups[g]; i++) {
                if (!w.hasNext()) {
                    return -1;
                }
                char c = w.peek();
                if (!((c >= '0' && c <= '9')
                        || (c >= 'a' && c <= 'f')
                        || (c >= 'A' && c <= 'F'))) {
                    return -1;
                }
                w.step();
                len++;
            }
            if (g < groups.length - 1) {
                if (!w.hasNext() || w.peek() != '-') {
                    return -1;
                }
                w.step();
                len++;
            }
        }

        return len == 0 ? -1 : len;
    };

    private final HashMap<String, Integer> tokenTypes;
    private final HashMap<TokenParser, Integer> tokenParsers;
    private final HashSet<Integer> needsBoundaries;
    private int maxLength = 0;
    private int count = 0;

    public TokenRegistry() {
        this.tokenTypes = new HashMap<>();
        this.tokenParsers = new HashMap<>();
        this.needsBoundaries = new HashSet<>();
    }

    public int registerToken(String token) {
        int ID = count++;
        tokenTypes.put(token, ID);
        int l = token.length();
        if (l > maxLength) {
            maxLength = l;
        }
        if (token.matches("[A-Za-z][A-Za-z0-9_]*")) {
            needsBoundaries.add(ID);
        }
        return ID;
    }

    public int registerMatch(TokenParser m) {
        int ID = count++;
        tokenParsers.put(m, ID);
        return ID;
    }

    public int get(CharView w) {
        int ID = -1;
        for (int len = Math.min(maxLength, w.remaining()); len >= 1; len--) {
            String candidate = w.get(len);
            Integer i = tokenTypes.get(candidate);
            if (i != null) {
                char before = w.charAtRelative(-1);
                char after = w.charAtRelative(len);
                if (!needsBoundaries.contains(i)
                        || ((before == 0 || !Character.isLetterOrDigit(before) && before != '_')
                        && (after == 0 || !Character.isLetterOrDigit(after) && after != '_'))) {
                    ID = i;
                    w.len = len;
                    break;
                }
            }
        }
        if (ID != -1) {
            return ID;
        }
        w.len = 0;
        for (Entry<TokenParser, Integer> m : tokenParsers.entrySet()) {
            CharView ww = new CharView(w);
            int l = m.getKey().match(ww);
            if (l != -1 && l > w.len) {
                w.len = l;
                ID = m.getValue();
            }
        }
        return ID;
    }
}
