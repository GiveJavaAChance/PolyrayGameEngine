package polyray.systems.parsing;

@FunctionalInterface
public interface TokenParser {

    public int match(CharView w);
}
