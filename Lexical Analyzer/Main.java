package A1;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

public class Main {

	public static void main(String[] args) {
		   // Change the file path as needed
		   String filePath = "src/A1/program.mylang"; 
		   try {
		       String input = Files.readString(Path.of(filePath));
		       List<Token> tokens = LexicalAnalyzer.tokenize(input);
		            
		            // Display errors
		            LexicalAnalyzer.displayErrors();
		            
		            // Display Tokens
		            System.out.println("\nTokens:");
		            tokens.forEach(System.out::println);
		            
		            // Display Symbol Table
		            System.out.println();
		            LexicalAnalyzer.symbolTable.printSymbolTable();
		            
		            // Optionally, display DFA representations for each token type
		             LexicalAnalyzer.displayTokensDFAStateCounts(tokens);
		            
		        } catch (IOException e) {
		            System.err.println("Error reading file: " + e.getMessage());
		        }
		    }

}
