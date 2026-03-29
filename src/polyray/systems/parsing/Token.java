package polyray.systems.parsing;

import java.util.Objects;

public class Token {

    public final String value;
    public final int type;

    public Token(String value, int type) {
        this.value = value;
        this.type = type;
    }

    @Override
    public boolean equals(Object other) {
        if (other == this) {
            return true;
        }
        if (other == null) {
            return false;
        }
        if (other instanceof Token t) {
            return this.type == t.type && this.value.equals(t.value);
        }
        return false;
    }

    @Override
    public int hashCode() {
        int hash = 7;
        hash = 79 * hash + Objects.hashCode(this.value);
        hash = 79 * hash + Objects.hashCode(this.type);
        return hash;
    }

    @Override
    public String toString() {
        return type + " : " + value;
    }
}
