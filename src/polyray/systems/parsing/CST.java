package polyray.systems.parsing;

import java.util.ArrayList;
import java.util.Stack;

public class CST {

    public final Token token;
    public final ArrayList<CST> children;

    public CST(Token token, ArrayList<CST> children) {
        this.token = token;
        this.children = children;
    }

    @Override
    public String toString() {
        if (token != null && children == null) {
            return token.type + ": \"" + token.value + "\"";
        }
        StringBuilder b = new StringBuilder();
        if (token != null) {
            b.append(token.type);
            b.append(":[\n");
        } else {
            b.append("[\n");
        }
        StringBuilder c = new StringBuilder();
        for (int i = 0; i < children.size(); i++) {
            c.append(children.get(i));
            if (i < children.size() - 1) {
                c.append(",\n");
            }
        }
        b.append(c.toString().indent(2));
        b.append("]");
        return b.toString();
    }

    public static final CST buildTree(ArrayList<Token> tokens, int... oc) {
        CST tree = new CST(null, new ArrayList<>());
        Stack<Token> stack = new Stack<>();
        for (int i = tokens.size() - 1; i >= 0; i--) {
            stack.push(tokens.get(i));
        }
        while (!stack.isEmpty()) {
            Token t = stack.peek();
            boolean noMatch = true;
            for (int i = 0; i < oc.length; i += 2) {
                if (t.type == oc[i]) {
                    tree.children.add(buildTree(oc[i], captureWhole(stack, oc[i], oc[i + 1]), oc));
                    noMatch = false;
                    break;
                }
            }
            if (noMatch) {
                tree.children.add(new CST(stack.pop(), null));
            }
        }
        return tree;
    }

    private static CST buildTree(int type, ArrayList<Token> tokens, int... oc) {
        CST tree = new CST(new Token(null, type), new ArrayList<>());
        Stack<Token> stack = new Stack<>();
        for (int i = tokens.size() - 1; i >= 0; i--) {
            stack.push(tokens.get(i));
        }
        while (!stack.isEmpty()) {
            Token t = stack.peek();
            boolean noMatch = true;
            for (int i = 0; i < oc.length; i += 2) {
                if (t.type == oc[i]) {
                    tree.children.add(buildTree(oc[i], captureWhole(stack, oc[i], oc[i + 1]), oc));
                    noMatch = false;
                    break;
                }
            }
            if (noMatch) {
                tree.children.add(new CST(stack.pop(), null));
            }
        }
        return tree;
    }

    private static ArrayList<Token> captureWhole(Stack<Token> stack, int open, int close) {
        ArrayList<Token> captured = new ArrayList<>();
        stack.pop();
        int depth = 1;
        while (depth > 0 && !stack.isEmpty()) {
            Token t = stack.pop();
            if (t.type == open) {
                depth++;
            } else if (t.type == close) {
                depth--;
                if (depth == 0) {
                    break;
                }
            }
            captured.add(t);
        }
        return captured;
    }
}
