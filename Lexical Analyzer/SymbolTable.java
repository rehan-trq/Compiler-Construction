package A1;

import java.util.*;

public class SymbolTable {
    
    public static class Symbol {
        private String name;
        private String type;
        private String scope;      // "global" or "local"
        private int scopeLevel;    // 1 for global, 2 for first nested block, etc.
        private int memoryLocation; // simulated memory location

        public Symbol(String name, String type, String scope, int scopeLevel, int memoryLocation) {
            this.name = name;
            this.type = type;
            this.scope = scope;
            this.scopeLevel = scopeLevel;
            this.memoryLocation = memoryLocation;
        }

        @Override
        public String toString() {
            return String.format("Name: %-20s | Type: %-20s | Scope: %-10s ", 
                    name, type, scope, scopeLevel, memoryLocation);
        }
    }
    
    // Each scope is a map from symbol names to Symbol objects.
    private Deque<Map<String, Symbol>> scopes;
    // List of all symbols declared (in insertion order).
    private List<Symbol> allSymbols;
    // Memory location counter (for simulation purposes).
    private int nextMemoryLocation;
    
    public SymbolTable() {
        scopes = new ArrayDeque<>();
        // Global scope is the bottom-most scope.
        scopes.push(new HashMap<>());
        allSymbols = new ArrayList<>();
        nextMemoryLocation = 1000; // starting memory location
    }
    
    // Enter a new (local) scope.
    public void enterScope() {
        scopes.push(new HashMap<>());
    }
    
    // Exit the current scope (global scope is never popped).
    public void exitScope() {
        if (scopes.size() > 1) {
            scopes.pop();
        } else {
            System.out.println("Warning: Attempted to exit the global scope.");
        }
    }
    
    // Returns the current scope level (1 = global, 2 = first nested, etc.).
    public int getCurrentScopeLevel() {
        return scopes.size();
    }
    
    // Helper: Check if an identifier is valid (only lowercase letters, digits, and underscores,
    // starting with a letter).
    private boolean isValidIdentifier(String name) {
        return name.matches("[a-z][a-z0-9_]*");
    }
    
    public boolean addSymbol(String name, String type, String scopeName) {
        Map<String, Symbol> currentScope = scopes.peek();
        if (currentScope.containsKey(name)) {
            System.out.println("Warning: Redeclaration of symbol '" + name + "' in the current scope.");
            return false;
        }
        // Only check identifiers for variable declarations.
        if ((type.equals("int") || type.equals("decimal") || type.equals("bool") || type.equals("char") ||
             type.equals("identifier")) && !isValidIdentifier(name)) {
            System.out.println("Error: Invalid identifier '" + name + "'. Identifiers must be lowercase.");
            return false;
        }
        int scopeLevel = getCurrentScopeLevel();
        Symbol sym = new Symbol(name, type, scopeName, scopeLevel, nextMemoryLocation++);
        currentScope.put(name, sym);
        allSymbols.add(sym);
        return true;
    }
    
    // Looks up a symbol by name (searching from the innermost to global scope).
    public Symbol lookup(String name) {
        for (Map<String, Symbol> scope : scopes) {
            if (scope.containsKey(name)) {
                return scope.get(name);
            }
        }
        return null;
    }
    
    // Prints all symbols grouped by their scope level.
    public void printSymbolTable() {
        System.out.println("\n--- Complete Symbol Table ---");
        // Group symbols by their scope level.
        Map<Integer, List<Symbol>> grouped = new TreeMap<>();
        for (Symbol sym : allSymbols) {
            grouped.computeIfAbsent(sym.scopeLevel, k -> new ArrayList<>()).add(sym);
        }
        for (Map.Entry<Integer, List<Symbol>> entry : grouped.entrySet()) {
            
            System.out.println("---------------------------------------------------------------");
            System.out.println("Name                 | Type                 | Scope     ");
            for (Symbol sym : entry.getValue()) {
                System.out.println(sym);
            }
            System.out.println();
        }
    }
}
