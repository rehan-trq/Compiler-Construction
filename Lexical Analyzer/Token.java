package A1;

public class Token {
    String type;
    String lexeme;
    int line;
    
    public Token(String type, String lexeme, int line) {
        this.type = type;
        this.lexeme = lexeme;
        this.line = line;
    }
    
    @Override
    public String toString() {
        return "Token(type=" + type + ", lexeme=" + lexeme + ", line=" + line + ")";
    }
}
