package polyray.systems.item;

import java.util.ArrayList;
import java.util.HashMap;

public class P2IN {

    private String name, content;
    private HashMap<String, P2IN> children = new HashMap<>();
    private boolean isLeaf = false;

    public P2IN(String str) {
        char[] chars = str.toCharArray();
        int idx = nextQuote(chars, str, 1);
        this.name = str.substring(1, idx);
        if (chars[idx + 2] == '{') { // The content is a list of objects
            ArrayList<String> strs = new ArrayList<>();
            while (true) {
                idx = nextQuote(chars, str, idx);
                if (idx == -1) {
                    break;
                }
                int start = idx;
                idx = nextQuote(chars, str, idx);
                if (chars[idx + 2] == '\"') {
                    idx = nextQuote(chars, str, idx + 2) + 1;
                    strs.add(str.substring(start, idx));
                } else {
                    int depth = 0;
                    boolean entered = false;
                    while (depth > 0 || !entered) {
                        char c = chars[idx++];
                        if (c == '\\') {
                            idx++;
                            continue;
                        }
                        switch (c) {
                            case '{' -> {
                                depth++;
                                entered = true;
                            }
                            case '}' -> {
                                depth--;
                            }
                        }
                    }
                    strs.add(str.substring(start, idx));
                }
            }
            for (String s : strs) {
                P2IN child = new P2IN(s);
                children.put(child.name, child);
            }
        } else { // The content is the value
            this.content = str.substring(idx + 3, str.length() - 1);
            this.isLeaf = true;
        }
    }

    private static int nextQuote(char[] chars, String str, int idx) {
        do {
            idx = str.indexOf("\"", idx + 1);
        } while (idx != -1 && chars[idx - 1] == '\\');
        return idx;
    }

    public String getName() {
        return this.name;
    }

    public String getContents() {
        return this.content;
    }

    public P2IN[] listChildren() {
        return children.values().toArray(P2IN[]::new);
    }

    public P2IN getContent(String name) {
        return children.get(name);
    }

    public boolean isLeaf() {
        return this.isLeaf;
    }
    
    @Override
    public String toString() {
        return toString(1);
    }
    
    private String toString(int indent) {
        if(this.isLeaf) {
            return this.name + " : " + this.content;
        }
        StringBuilder b = new StringBuilder(this.name);
        P2IN[] ch = listChildren();
        for (int i = 0; i < ch.length; i++) {
            b.append('\n');
            b.append("    ".repeat(indent));
            b.append(ch[i].toString(indent + 1));
        }
        return b.toString();
    }
}
