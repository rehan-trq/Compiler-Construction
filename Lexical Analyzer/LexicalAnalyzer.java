package A1;

import java.util.*;
import java.util.regex.*;

public class LexicalAnalyzer {
    // Define token patterns using a LinkedHashMap to preserve insertion order.
    public static final Map<String, String> TOKEN_PATTERNS = new LinkedHashMap<>();
    // Order in which token patterns will be checked in the tokenization loop.
    public static final List<String> TOKEN_TYPES_ORDER = new ArrayList<>();
    
    // A static symbol table instance.
    public static SymbolTable symbolTable = new SymbolTable();
    // Holds a pending data type when a KEYWORD (like int, decimal, bool, or char) is encountered.
    private static String lastDatatype = null;
    
    static {
        // Multi-line and single-line comments are handled separately.
        // String literals.
        TOKEN_PATTERNS.put("STRING_LITERAL", "\"(\\\\.|[^\"])*\"");
        // Keywords for data types and control flow.
        TOKEN_PATTERNS.put("KEYWORD", "\\b(int|decimal|bool|char|if|else|true|false|read|write)\\b");
        // Operators: note that "=" is matched before single-character operators.
        TOKEN_PATTERNS.put("OPERATOR", "=|>|\\+|\\-|\\*|/|%|\\^");
        // Delimiters.
        TOKEN_PATTERNS.put("DELIMITER", "[;(){}]");
        // Decimal numbers: one or more digits, a dot, and 1–5 digits.
        TOKEN_PATTERNS.put("DECIMAL", "\\b\\d+\\.\\d{1,5}\\b");
        // Integer numbers.
        TOKEN_PATTERNS.put("INTEGER", "\\b\\d+\\b");
        // Character literals.
        TOKEN_PATTERNS.put("CHAR_LITERAL", "'(\\\\.|[^'])'");
        // Identifiers: must start with a lowercase letter, followed by lowercase letters, digits, or underscores.
        TOKEN_PATTERNS.put("IDENTIFIER", "\\b[a-z][a-z0-9_]*\\b");
        // Whitespace (used only for skipping).
        TOKEN_PATTERNS.put("WHITESPACE", "\\s+");
        
        // Define the order for token matching.
        TOKEN_TYPES_ORDER.add("STRING_LITERAL");
        TOKEN_TYPES_ORDER.add("KEYWORD");
        TOKEN_TYPES_ORDER.add("OPERATOR");
        TOKEN_TYPES_ORDER.add("DELIMITER");
        TOKEN_TYPES_ORDER.add("DECIMAL");
        TOKEN_TYPES_ORDER.add("INTEGER");
        TOKEN_TYPES_ORDER.add("CHAR_LITERAL");
        TOKEN_TYPES_ORDER.add("IDENTIFIER");
        // WHITESPACE is handled separately.
    }
    
    public static List<Token> tokenize(String code) {
        List<Token> tokens = new ArrayList<>();
        
        // First, process multi-line comments.
        Pattern mlcPattern = Pattern.compile("/\\*(?:.|[\\r\\n])*?\\*/");
        Matcher mlcMatcher = mlcPattern.matcher(code);
        List<int[]> mlcRanges = new ArrayList<>();
        while (mlcMatcher.find()) {
            String comment = mlcMatcher.group();
            int commentStartLine = code.substring(0, mlcMatcher.start()).split("\\n").length;
            int commentEndLine = code.substring(0, mlcMatcher.end()).split("\\n").length;
            tokens.add(new Token("COMMENT", comment, commentStartLine));
            mlcRanges.add(new int[]{commentStartLine, commentEndLine});
        }
        
        // Process the source code line by line.
        String[] lines = code.split("\\n");
        for (int i = 0; i < lines.length; i++) {
            String line = lines[i];
            int currentLine = i + 1;
            // Skip lines that fall within a multi-line comment.
            boolean inMultiLineComment = false;
            for (int[] range : mlcRanges) {
                if (currentLine >= range[0] && currentLine <= range[1]) {
                    inMultiLineComment = true;
                    break;
                }
            }
            if (inMultiLineComment) {
                continue;
            }
            
            // Check for a single-line comment.
            Pattern slcPattern = Pattern.compile("//.*");
            Matcher slcMatcher = slcPattern.matcher(line);
            if (slcMatcher.find()) {
                String comment = slcMatcher.group();
                tokens.add(new Token("COMMENT", comment, currentLine));
                // Remove the comment part from the line before further tokenization.
                line = line.substring(0, slcMatcher.start());
            }
            
            int pos = 0;
            while (pos < line.length()) {
                // Skip whitespace.
                Pattern wsPattern = Pattern.compile(TOKEN_PATTERNS.get("WHITESPACE"));
                Matcher wsMatcher = wsPattern.matcher(line.substring(pos));
                if (wsMatcher.find() && wsMatcher.start() == 0) {
                    pos += wsMatcher.end();
                    continue;
                }
                
                boolean matched = false;
                for (String tokenType : TOKEN_TYPES_ORDER) {
                    String patternStr = TOKEN_PATTERNS.get(tokenType);
                    Pattern pattern = Pattern.compile(patternStr);
                    Matcher matcher = pattern.matcher(line.substring(pos));
                    if (matcher.find() && matcher.start() == 0) {
                        String lexeme = matcher.group();
                        
                        // For delimiters, update scope if needed.
                        if (tokenType.equals("DELIMITER")) {
                            if (lexeme.equals("{")) {
                                symbolTable.enterScope();
                            } else if (lexeme.equals("}")) {
                                symbolTable.exitScope();
                            }
                        }
                        
                        // If the token is a KEYWORD that represents a data type, record it.
                        if (tokenType.equals("KEYWORD")) {
                            if (lexeme.equals("int") || lexeme.equals("decimal") ||
                                lexeme.equals("bool") || lexeme.equals("char")) {
                                lastDatatype = lexeme;
                            }
                        }
                        
                        // If an identifier immediately follows a datatype, add it to the symbol table.
                        if (tokenType.equals("IDENTIFIER") && lastDatatype != null) {
                            String scopeName = (symbolTable.getCurrentScopeLevel() == 1) ? "global" : "local";
                            symbolTable.addSymbol(lexeme, lastDatatype, scopeName);
                            lastDatatype = null;
                        }
                        
                        tokens.add(new Token(tokenType, lexeme, currentLine));
                        pos += lexeme.length();
                        matched = true;
                        break;
                    }
                }
                if (!matched) {
                    ErrorHandler.addError(currentLine, "Unrecognized token near: '" + line.charAt(pos) + "'");
                    pos++;
                }
            }
        }
        return tokens;
    }
    
    public static void displayErrors() {
        ErrorHandler.displayErrors();
    }
    
    // Build symbol table entries from tokens that represent constants, I/O, strings, comments, and arithmetic operators.
    public static void buildSymbolTableFromTokens(List<Token> tokens) {
        Set<String> addedConstants = new HashSet<>();
        for (Token token : tokens) {
            // Add string constants.
            if (token.type.equals("STRING_LITERAL")) {
                if (!addedConstants.contains(token.lexeme)) {
                    symbolTable.addSymbol(token.lexeme, "string constant", "global");
                    addedConstants.add(token.lexeme);
                }
            }
            // Add comments.
            else if (token.type.equals("COMMENT")) {
                if (!addedConstants.contains(token.lexeme)) {
                    symbolTable.addSymbol(token.lexeme, "comment", "global");
                    addedConstants.add(token.lexeme);
                }
            }
            // Add arithmetic operators.
            else if (token.type.equals("OPERATOR") && "+-*/%^".contains(token.lexeme)) {
                if (!addedConstants.contains(token.lexeme)) {
                    symbolTable.addSymbol(token.lexeme, "arithmetic operator", "global");
                    addedConstants.add(token.lexeme);
                }
            }
            // Add I/O operations.
            else if (token.type.equals("KEYWORD") && (token.lexeme.equals("print") || 
                                                       token.lexeme.equals("read")  || 
                                                       token.lexeme.equals("input") || 
                                                       token.lexeme.equals("write"))) {
                if (!addedConstants.contains(token.lexeme)) {
                    symbolTable.addSymbol(token.lexeme, "io", "global");
                    addedConstants.add(token.lexeme);
                }
            }
            // For numeric constants, you might choose to add them as well:
            else if (token.type.equals("INTEGER") || token.type.equals("DECIMAL")) {
                if (!addedConstants.contains(token.lexeme)) {
                    // Determine scope: if it appears in the global area (for example, as a literal constant) mark it global.
                    // This simple example marks all numeric constants as global.
                    symbolTable.addSymbol(token.lexeme, "numeric constant", "global");
                    addedConstants.add(token.lexeme);
                }
            }
        }
    }
    
    // Build and display the combined DFA state counts for only the token types actually encountered.
    public static void displayTokensDFAStateCounts(List<Token> tokens) {
        // Collect the unique token types that appear in the token stream.
        Set<String> encounteredTypes = new HashSet<>();
        for (Token t : tokens) {
            encounteredTypes.add(t.type);
        }
        dk.brics.automaton.Automaton combined = null;
        for (String tokenType : encounteredTypes) {
            String regex = TOKEN_PATTERNS.get(tokenType);
            if (regex == null) continue; // Skip if no pattern exists (e.g., for UNKNOWN)
            String automatonRegex = Conversion.convertToAutomatonRegex(regex);
            try {
                dk.brics.automaton.RegExp re = new dk.brics.automaton.RegExp(automatonRegex);
                dk.brics.automaton.Automaton dfa = re.toAutomaton();
                dfa.determinize();
                dfa.minimize();
                if (combined == null) {
                    combined = dfa;
                } else {
                    combined = combined.union(dfa);
                }
            } catch (IllegalArgumentException e) {
                System.out.println("Skipping DFA generation for token type: " + tokenType +
                                   " due to regex error: " + e.getMessage());
            }
        }
        if (combined != null) {
            combined.determinize();
            combined.minimize();
            System.out.println("\nCombined DFA State and Transition Counts (for actual tokens encountered):");
            Conversion.printDFAStates(combined);
        } else {
            System.out.println("No valid DFAs generated from the tokens encountered.");
        }
    }
}
