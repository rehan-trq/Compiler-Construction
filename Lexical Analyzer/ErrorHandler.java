package A1;

import java.util.ArrayList;
import java.util.List;

public class ErrorHandler {
    private static final List<String> errors = new ArrayList<>();

    public static void addError(int line, String message) {
        errors.add("Error at line " + line + ": " + message);
    }

    public static void displayErrors() {
        if (errors.isEmpty()) {
            System.out.println("\nNo lexical errors found.");
        } else {
            System.out.println("\n-- Lexical Errors --");
            for (String error : errors) {
                System.out.println(error);
            }
        }
    }

    public static boolean hasErrors() {
        return !errors.isEmpty();
    }
}
