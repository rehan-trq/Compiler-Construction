package A1;

import java.util.Set;

import dk.brics.automaton.*;

public class Conversion {
    public static String convertToAutomatonRegex(String javaRegex) {
        // Remove word boundaries since automaton regex does not support \b
        String automatonRegex = javaRegex.replace("\\b", "");

        // Convert Java regex patterns to equivalent DFA-compatible patterns
        automatonRegex = automatonRegex.replace("\\d", "[0-9]"); // Digits
        automatonRegex = automatonRegex.replace("\\w", "[a-zA-Z0-9_]");
        automatonRegex = automatonRegex.replace("\\s", "[ \\t\\n\\r\\f]"); // Whitespace
        automatonRegex = automatonRegex.replace("\\S", "[^ \\t\\n\\r\\f]"); // Non-whitespace
        
        // Remove non-capturing group syntax (not supported in automaton regex)
        automatonRegex = automatonRegex.replaceAll("\\(\\?:.*?\\)", "");

        // Ensure optional quantifiers like `*?` are converted to `*`
        automatonRegex = automatonRegex.replaceAll("\\*\\?", "*");
        automatonRegex = automatonRegex.replaceAll("\\+\\?", "+");
        automatonRegex = automatonRegex.replaceAll("\\?\\?", "?");

        // Handle simple grouped choices like (a|b) converting to [ab]
        automatonRegex = automatonRegex.replaceAll("\\((\\w)\\|(\\w)\\)", "[$1$2]");

        return automatonRegex;
    }

    public static void printDFAStates(Automaton dfa) {
        Set<State> states = dfa.getStates();
        int totalTransitions = 0;
        System.out.println("DFA States and Transitions:");
        for (State state : states) {
            System.out.println("State " + state + (state.isAccept() ? " [Accepting]" : " [Rejecting]"));
            for (Transition t : state.getTransitions()) {
                System.out.println("  Transition: '" + t.getMin() + "' → State " + t.getDest());
                totalTransitions++;
            }
            System.out.println();
        }
        System.out.println("Total DFA states: " + states.size());
        System.out.println("Total DFA transitions: " + totalTransitions);
    }
}
