package polyray.systems.parsing;

public class CharView {

    private final char[] chars;
    private int pos;
    public int len;

    public CharView(char[] chars, int pos) {
        this.chars = chars;
        this.pos = pos;
    }

    public CharView(CharView w) {
        this.chars = w.chars;
        this.pos = w.pos;
    }

    public String get(int len) {
        return new String(chars, pos, len);
    }

    public char peek() {
        return chars[pos];
    }

    public char charAtRelative(int off) {
        int idx = pos + off;
        if (idx < 0 || idx >= chars.length) {
            return 0;
        }
        return chars[idx];
    }

    public boolean nextRange(char... ranges) {
        char c = peek();
        boolean contains = false;
        for (int i = 0; i < ranges.length; i += 2) {
            contains |= c >= ranges[i] && c <= ranges[i + 1];
        }
        return contains;
    }

    public void step() {
        pos++;
    }

    public void step(int amt) {
        pos += amt;
    }

    public boolean hasNext() {
        return pos < chars.length;
    }

    public int remaining() {
        return chars.length - pos;
    }

    public void ex(String msg) {
        throw new RuntimeException(msg + " at position " + pos);
    }
}
