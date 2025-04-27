#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXIMUMPRODUCTIONS 100
#define MAXIMUMSYMBOLS 100
#define MAXIMUMRHSSIZE 20
#define MAXIMUMSYMBOLLENGTH 10
#define MAXIMUMSTACKSIZE 100
#define MAXIMUMINPUTLENGTH 256

// Structure to represent a production

typedef struct 
{
    char lhs[MAXIMUMSYMBOLLENGTH];
    char rhs[MAXIMUMRHSSIZE][MAXIMUMSYMBOLLENGTH][MAXIMUMSYMBOLLENGTH];
    int rhscount;
    int rhssymbols[MAXIMUMRHSSIZE];
} 
Production;


typedef struct 
{
    char items[MAXIMUMSTACKSIZE][MAXIMUMSYMBOLLENGTH];
    int top;
} 
ParsingStack;

// Structure to represent the grammar

typedef struct 
{
    Production productions[MAXIMUMPRODUCTIONS];
    int productioncount;
    char nonterminals[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH];
    int nonterminalcount;
    char terminals[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH];
    int terminalcount;
    char startsymbol[MAXIMUMSYMBOLLENGTH];
} 
Grammar;

FILE* out;

// Function to read grammar from file

Grammar readfromfile(const char* filename);

// Function to perform left factoring

Grammar performleftfactoring(Grammar g);

// Function to remove left recursion

Grammar removeleftrecursion(Grammar g);

// Function to compute FIRST sets

void calculatefirstsets(Grammar g, char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int first_count[MAXIMUMSYMBOLS]);

// Function to compute FOLLOW sets

void calculatefollowsets(Grammar g, char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int first_count[MAXIMUMSYMBOLS], char follow_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int follow_count[MAXIMUMSYMBOLS]);

// Function to construct LL(1) parsing table

void generateparsingtable(Grammar g, char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int first_count[MAXIMUMSYMBOLS], char follow_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int follow_count[MAXIMUMSYMBOLS], int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS]);

// Function to print grammar

void printgrammar(Grammar g);

// Function to print parsing table

void printparsingtable(Grammar g, int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS]);

// Utility functions

int checknonterminal(char* symbol);
int getindexofnonterminal(Grammar g, char* symbol);
int getindexofterminal(Grammar g, char* symbol);
int checkepsilon(char set[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int count);
void addtoset(char set[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int* count, char* symbol);

// New functions for Assignment 3

void initializestack(ParsingStack* stack);
void push(ParsingStack* stack, char* symbol);
void pop(ParsingStack* stack);
char* top(ParsingStack* stack);
int isempty(ParsingStack* stack);

//void printstack(ParsingStack* stack);

void printstack(ParsingStack* stack, FILE* file);
void parseinput(Grammar g, int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS], const char* inputfilename);
int parsestring(Grammar g, int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS], char* input, int lineNo, FILE* outputFile);


int main() 
{
    // Read grammar from file

    out = fopen("D:\\Semester 6\\CC\\A3\\22i0965-22i2427-A\\output.txt", "w");
	if (!out) 
    {
		printf("Failed to open output.txt\n");
		return 1;
	}

    Grammar g = readfromfile("grammar.txt");

    printf("Original CFG:\n");
    fprintf(out, "Original CFG:\n");
    printgrammar(g);
   
    // Perform left factoring

    Grammar leftfactoredgrammar = performleftfactoring(g);
    printf("\nGrammar after Left Factoring:\n");
    fprintf(out, "\nCFG after Left Factoring:\n");
    printgrammar(leftfactoredgrammar);
   
    // Remove left recursion

    Grammar grammarwoleftrecursion = removeleftrecursion(leftfactoredgrammar);
    printf("\nGrammar after Left Recursion Removal:\n");
    fprintf(out, "\nCFG after Left Recursion Removal:\n");
    printgrammar(grammarwoleftrecursion);
   
    // Compute FIRST sets

    char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH];
    int first_count[MAXIMUMSYMBOLS] = {0};
    calculatefirstsets(grammarwoleftrecursion, first_sets, first_count);
   
    // Print FIRST sets

    printf("\nFirst Sets:\n");
    fprintf(out, "\nFirst Sets:\n");
    for (int i = 0; i < grammarwoleftrecursion.nonterminalcount; i++) 
    {
        printf("First (%s) = { ", grammarwoleftrecursion.nonterminals[i]);
        fprintf(out, "First (%s) = { ", grammarwoleftrecursion.nonterminals[i]);

        for (int j = 0; j < first_count[i]; j++) 
        {
            printf("%s ", first_sets[i][j]);
            fprintf(out, "%s ", first_sets[i][j]);
            if (j < first_count[i] - 1) printf(", ");
        }

        printf("}\n");
        fprintf(out, "}\n");
    }
   
    // Compute FOLLOW sets

    char follow_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH];
    int follow_count[MAXIMUMSYMBOLS] = {0};
    calculatefollowsets(grammarwoleftrecursion, first_sets, first_count, follow_sets, follow_count);
   
    // Print FOLLOW sets

    printf("\nFollow Sets:\n");
    fprintf(out, "\nFollow Sets:\n");
    for (int i = 0; i < grammarwoleftrecursion.nonterminalcount; i++) 
    {
        printf("Follow (%s) = { ", grammarwoleftrecursion.nonterminals[i]);
        fprintf(out, "Follow (%s) = { ", grammarwoleftrecursion.nonterminals[i]);
        for (int j = 0; j < follow_count[i]; j++) 
        {
            printf("%s ", follow_sets[i][j]);
            fprintf(out, "%s ", follow_sets[i][j]);
            if (j < follow_count[i] - 1) printf(", ");
        }
        printf("}\n");
        fprintf(out, "}\n");
    }
   
    // Construct LL(1) parsing table

    int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS];
    memset(parsing_table, -1, sizeof(parsing_table));
    generateparsingtable(grammarwoleftrecursion, first_sets, first_count, follow_sets, follow_count, parsing_table);
    printparsingtable(grammarwoleftrecursion, parsing_table);
    
    
    // Assignment 3 - Parse input file using the LL(1) parsing table
    
    parseinput(grammarwoleftrecursion, parsing_table, "input.txt");

    fclose(out);
    return 0;
}

Grammar readfromfile(const char* filename) 
{
    Grammar g;
    g.productioncount = 0;
    g.nonterminalcount = 0;
    g.terminalcount = 0;
   
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error opening file\n");
        exit(1);
    }
   
    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline and skip empty lines
        line[strcspn(line, "\n")] = '\0';
        if (strlen(line) == 0) continue;
       
        // Find the "->" arrow in the line
        char *arrow = strstr(line, "->");
        if (!arrow) continue;
       
        // Split into LHS and RHS parts
        *arrow = '\0';
        arrow += 2;  // Skip past "->"
       
        // Trim LHS
        char *lhs = line;
        while (isspace(*lhs)) lhs++;
        char *end = lhs + strlen(lhs) - 1;
        while (end >= lhs && isspace(*end)) {
            *end = '\0';
            end--;
        }
       
        // Set production LHS and update non-terminals list
        strcpy(g.productions[g.productioncount].lhs, lhs);
        int found = 0;
        for (int i = 0; i < g.nonterminalcount; i++) {
            if (strcmp(g.nonterminals[i], lhs) == 0) {
                found = 1;
                break;
            }
        }
        if (!found) {
            strcpy(g.nonterminals[g.nonterminalcount], lhs);
            g.nonterminalcount++;
        }
       
        // For the first production, set start symbol
        if (g.productioncount == 0) {
            strcpy(g.startsymbol, lhs);
        }
       
        // Process the RHS part manually
        g.productions[g.productioncount].rhscount = 0;
        char *rhs = arrow;
        // Trim leading whitespace on RHS
        while (*rhs && isspace(*rhs)) rhs++;
       
        // Loop over alternatives separated by '|'
        char *alt = rhs;
        while (alt && *alt) {
            // Look for the next pipe
            char *pipe = strchr(alt, '|');
            if (pipe) {
                *pipe = '\0'; // Terminate current alternative
            }
           
            // Trim alternative
            while (isspace(*alt)) alt++;
            char *alt_end = alt + strlen(alt) - 1;
            while (alt_end >= alt && isspace(*alt_end)) {
                *alt_end = '\0';
                alt_end--;
            }
           
            // Tokenize this alternative by spaces to get individual symbols
            int symbol_index = 0;
            char temp[256];
            strcpy(temp, alt);
            char *token = strtok(temp, " ");
            while (token) {
                // Copy the token into the production alternative
                strcpy(g.productions[g.productioncount].rhs[g.productions[g.productioncount].rhscount][symbol_index], token);
                // Update non-terminals/terminals lists
                if (isupper(token[0])) {
                    int found = 0;
                    for (int i = 0; i < g.nonterminalcount; i++) {
                        if (strcmp(g.nonterminals[i], token) == 0) {
                            found = 1;
                            break;
                        }
                    }
                    if (!found) {
                        strcpy(g.nonterminals[g.nonterminalcount], token);
                        g.nonterminalcount++;
                    }
                } else {
                    if (strcmp(token, "ε") != 0) {
                        int found = 0;
                        for (int i = 0; i < g.terminalcount; i++) {
                            if (strcmp(g.terminals[i], token) == 0) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            strcpy(g.terminals[g.terminalcount], token);
                            g.terminalcount++;
                        }
                    }
                }
                symbol_index++;
                token = strtok(NULL, " ");
            }
            g.productions[g.productioncount].rhssymbols[g.productions[g.productioncount].rhscount] = symbol_index;
            g.productions[g.productioncount].rhscount++;
           
            if (pipe) {
                alt = pipe + 1;
            } else {
                break;
            }
        }
       
        g.productioncount++;
    }
   
    fclose(file);
    return g;
}

int longest_common_prefix_tokens(char alt1[MAXIMUMSYMBOLLENGTH][MAXIMUMSYMBOLLENGTH],int alt1_len,char alt2[MAXIMUMSYMBOLLENGTH][MAXIMUMSYMBOLLENGTH],int alt2_len)
{
    int min_len = (alt1_len < alt2_len) ? alt1_len : alt2_len;
    int i;
    for (i = 0; i < min_len; i++) {
        if (strcmp(alt1[i], alt2[i]) != 0) {
            break;
        }
    }
    return i; // number of matching tokens
}

void left_factor_production(Grammar *g, Production *p) 
{
    // We'll create a temporary production to hold the new alternatives.
    Production newProd;
    strcpy(newProd.lhs, p->lhs);
    newProd.rhscount = 0;
   
    // Create an array to mark which alternatives have been processed.
    int processed[MAXIMUMRHSSIZE] = {0};
   
    // For each alternative in p:
    for (int i = 0; i < p->rhscount; i++) {
        if (processed[i] || p->rhssymbols[i] == 0)
            continue;
        // Group alternatives that share the same first token.
        char firstToken[MAXIMUMSYMBOLLENGTH];
        strcpy(firstToken, p->rhs[i][0]);
       
        // Start a new group with alternative i.
        int groupCount = 1;
        processed[i] = 1;
       
        // Temporary storage for suffixes in the group.
        // Each suffix is a sequence of tokens from index 1 onward.
        char suffixes[MAXIMUMRHSSIZE][MAXIMUMSYMBOLLENGTH][MAXIMUMSYMBOLLENGTH];
        int suffixLen[MAXIMUMRHSSIZE] = {0};
       
        // Save suffix for alternative i.
        suffixLen[0] = p->rhssymbols[i] - 1;
        for (int k = 1; k < p->rhssymbols[i]; k++) {
            strcpy(suffixes[0][k-1], p->rhs[i][k]);
        }
       
        // Check the remaining alternatives.
        for (int j = i+1; j < p->rhscount; j++) {
            if (p->rhssymbols[j] == 0) continue;
            if (strcmp(p->rhs[j][0], firstToken) == 0) {
                // Same first token – add to the group.
                processed[j] = 1;
                suffixLen[groupCount] = p->rhssymbols[j] - 1;
                for (int k = 1; k < p->rhssymbols[j]; k++) {
                    strcpy(suffixes[groupCount][k-1], p->rhs[j][k]);
                }
                groupCount++;
            }
        }
       
        if (groupCount >= 2) {
            // Factor this group out.
            // Create a new non-terminal for the factored suffix.
            char new_nt[MAXIMUMSYMBOLLENGTH];
            sprintf(new_nt, "%s'", p->lhs);
            // Ensure uniqueness by appending additional primes if needed.
            int unique = 0;
            while (!unique) {
                unique = 1;
                for (int x = 0; x < g->nonterminalcount; x++) {
                    if (strcmp(g->nonterminals[x], new_nt) == 0) {
                        strcat(new_nt, "'");
                        unique = 0;
                        break;
                    }
                }
            }
            // Add new_nt to grammar's non-terminals.
            strcpy(g->nonterminals[g->nonterminalcount++], new_nt);
           
            // In the original production, add one alternative: firstToken followed by new_nt.
            strcpy(newProd.rhs[newProd.rhscount][0], firstToken);
            strcpy(newProd.rhs[newProd.rhscount][1], new_nt);
            newProd.rhssymbols[newProd.rhscount] = 2;
            newProd.rhscount++;
           
            // Create a new production for new_nt.
            Production newProd2;
            strcpy(newProd2.lhs, new_nt);
            newProd2.rhscount = 0;
            // For each alternative in the group, add its suffix as an alternative.
            for (int gIdx = 0; gIdx < groupCount; gIdx++) {
                int sLen = suffixLen[gIdx];
                int altIndex = newProd2.rhscount;
                if (sLen == 0) {
                    // If no suffix, add epsilon.
                    strcpy(newProd2.rhs[altIndex][0], "ε");
                    newProd2.rhssymbols[altIndex] = 1;
                } else {
                    for (int t = 0; t < sLen; t++) {
                        strcpy(newProd2.rhs[altIndex][t], suffixes[gIdx][t]);
                    }
                    newProd2.rhssymbols[altIndex] = sLen;
                }
                newProd2.rhscount++;
            }
            // Add newProd2 to grammar.
            g->productions[g->productioncount++] = newProd2;
        } else {
            // Only one alternative had this first token: copy it unchanged.
            // Find the alternative index (which is i).
            for (int t = 0; t < p->rhssymbols[i]; t++) {
                strcpy(newProd.rhs[newProd.rhscount][t], p->rhs[i][t]);
            }
            newProd.rhssymbols[newProd.rhscount] = p->rhssymbols[i];
            newProd.rhscount++;
        }
    }
   
    // Replace p with the new production (which now has factored alternatives).
    *p = newProd;
}

Grammar performleftfactoring(Grammar g) 
{
    int changed = 1;
    while (changed) {
        changed = 0;
        for (int i = 0; i < g.productioncount; i++) {
            int oldCount = g.productions[i].rhscount;
            left_factor_production(&g, &g.productions[i]);
            if (g.productions[i].rhscount != oldCount)
                changed = 1;
        }
    }
    return g;

}

// Revised removeleftrecursion that handles productions with no non-left-recursive alternative.
Grammar removeleftrecursion(Grammar g) 
{
    Grammar result;
    result.productioncount = 0;
    result.nonterminalcount = g.nonterminalcount;
    result.terminalcount = g.terminalcount;
    strcpy(result.startsymbol, g.startsymbol);
   
    // Copy existing non-terminals and terminals.
    for (int i = 0; i < g.nonterminalcount; i++) {
        strcpy(result.nonterminals[i], g.nonterminals[i]);
    }
    for (int i = 0; i < g.terminalcount; i++) {
        strcpy(result.terminals[i], g.terminals[i]);
    }
   
    // Process each production (assumed one production per non-terminal).
    for (int i = 0; i < g.productioncount; i++) {
        Production prod = g.productions[i];
        char A[MAXIMUMSYMBOLLENGTH];
        strcpy(A, prod.lhs);
       
        // Temporary storage for alternatives:
        // alpha: non-left-recursive alternatives.
        // beta: left-recursive alternatives (with A as the first token).
        char alpha[MAXIMUMRHSSIZE][MAXIMUMSYMBOLLENGTH][MAXIMUMSYMBOLLENGTH];
        int alpha_count = 0;
        int alpha_symbol_count[MAXIMUMRHSSIZE] = {0};
       
        char beta[MAXIMUMRHSSIZE][MAXIMUMSYMBOLLENGTH][MAXIMUMSYMBOLLENGTH];
        int beta_count = 0;
        int beta_symbol_count[MAXIMUMRHSSIZE] = {0};
       
        // Separate alternatives.
        for (int j = 0; j < prod.rhscount; j++) {
            if (prod.rhssymbols[j] > 0 && strcmp(prod.rhs[j][0], A) == 0) {
                // Left recursive alternative: store its suffix (tokens after A).
                beta_symbol_count[beta_count] = prod.rhssymbols[j] - 1;
                for (int k = 1; k < prod.rhssymbols[j]; k++) {
                    strcpy(beta[beta_count][k - 1], prod.rhs[j][k]);
                }
                beta_count++;
            } else {
                // Non-left-recursive alternative.
                alpha_symbol_count[alpha_count] = prod.rhssymbols[j];
                for (int k = 0; k < prod.rhssymbols[j]; k++) {
                    strcpy(alpha[alpha_count][k], prod.rhs[j][k]);
                }
                alpha_count++;
            }
        }
       
        if (beta_count > 0) {
            // Left recursion exists for A.
            // Generate a new non-terminal name for the left-recursive part.
            char new_nt[MAXIMUMSYMBOLLENGTH];
            sprintf(new_nt, "%s'", A);
            // Ensure uniqueness: if new_nt is already present, append another prime.
            while(getindexofnonterminal(result, new_nt) != -1) {
                strcat(new_nt, "'");
            }
            // Add new_nt to result's non-terminals.
            strcpy(result.nonterminals[result.nonterminalcount], new_nt);
            result.nonterminalcount++;
           
            // CASE 1: If at least one non-left-recursive alternative exists.
            if (alpha_count > 0) {
                Production newProd;
                strcpy(newProd.lhs, A);
                newProd.rhscount = 0;
                for (int j = 0; j < alpha_count; j++) {
                    int count = alpha_symbol_count[j];
                    for (int k = 0; k < count; k++) {
                        strcpy(newProd.rhs[newProd.rhscount][k], alpha[j][k]);
                    }
                    // Append new_nt at the end.
                    strcpy(newProd.rhs[newProd.rhscount][count], new_nt);
                    newProd.rhssymbols[newProd.rhscount] = count + 1;
                    newProd.rhscount++;
                }
                result.productions[result.productioncount] = newProd;
                result.productioncount++;
            } else {
                // CASE 2: No non-left-recursive alternative.
                Production newProd;
                strcpy(newProd.lhs, A);
                newProd.rhscount = 1;
                int count = beta_symbol_count[0]; // Use the first beta alternative.
                for (int k = 0; k < count; k++) {
                    strcpy(newProd.rhs[0][k], beta[0][k]);
                }
                // Append new_nt.
                strcpy(newProd.rhs[0][count], new_nt);
                newProd.rhssymbols[0] = count + 1;
                result.productions[result.productioncount] = newProd;
                result.productioncount++;
            }
           
            // Create production for the new non-terminal new_nt.
            Production newProd2;
            strcpy(newProd2.lhs, new_nt);
            newProd2.rhscount = 0;
            for (int j = 0; j < beta_count; j++) {
                int count = beta_symbol_count[j];
                for (int k = 0; k < count; k++) {
                    strcpy(newProd2.rhs[newProd2.rhscount][k], beta[j][k]);
                }
                // Append new_nt at the end for recursion.
                strcpy(newProd2.rhs[newProd2.rhscount][count], new_nt);
                newProd2.rhssymbols[newProd2.rhscount] = count + 1;
                newProd2.rhscount++;
            }
            // Add an alternative for epsilon.
            strcpy(newProd2.rhs[newProd2.rhscount][0], "ε");
            newProd2.rhssymbols[newProd2.rhscount] = 1;
            newProd2.rhscount++;
           
            result.productions[result.productioncount] = newProd2;
            result.productioncount++;
        } else {
            // No left recursion: copy the production as is.
            result.productions[result.productioncount] = prod;
            result.productioncount++;
        }
    }
   
    return result;
}

void calculatefirstsets(Grammar g, char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int first_count[MAXIMUMSYMBOLS]) 
{
    int i, j, k, t;
    // Initialize FIRST sets for all non-terminals to empty.
    for (i = 0; i < g.nonterminalcount; i++) {
        first_count[i] = 0;
    }
   
    // For terminals, we store their FIRST set in the indices after non-terminals (if needed)
    for (i = 0; i < g.terminalcount; i++) {
        strcpy(first_sets[g.nonterminalcount + i][0], g.terminals[i]);
        first_count[g.nonterminalcount + i] = 1;
    }
   
    int changed = 1;
    while(changed) {
        changed = 0;
        // Process each production in the grammar.
        for (i = 0; i < g.productioncount; i++) {
            Production p = g.productions[i];
            // Get the index for the LHS non-terminal.
            int lhs_index = getindexofnonterminal(g, p.lhs);
            if (lhs_index == -1) continue;
           
            // Process each alternative for this production.
            for (j = 0; j < p.rhscount; j++) {
                // If the alternative is exactly "epsilon", add it.
                if (p.rhssymbols[j] == 1 && strcmp(p.rhs[j][0], "ε") == 0) {
                    int exists = 0;
                    for (t = 0; t < first_count[lhs_index]; t++) {
                        if (strcmp(first_sets[lhs_index][t], "ε") == 0) {
                            exists = 1;
                            break;
                        }
                    }
                    if (!exists) {
                        strcpy(first_sets[lhs_index][first_count[lhs_index]++], "ε");
                        changed = 1;
                    }
                    continue;
                }
               
                // Process the symbols in the alternative left-to-right.
                int allCanBeEpsilon = 1;
                for (k = 0; k < p.rhssymbols[j]; k++) {
                    char *symbol = p.rhs[j][k];
                    // Check if the symbol is terminal or non-terminal by using our grammar.
                    if (getindexofnonterminal(g, symbol) == -1) {
                        // symbol is a terminal; add it and stop.
                        int exists = 0;
                        for (t = 0; t < first_count[lhs_index]; t++) {
                            if (strcmp(first_sets[lhs_index][t], symbol) == 0) {
                                exists = 1;
                                break;
                            }
                        }
                        if (!exists) {
                            strcpy(first_sets[lhs_index][first_count[lhs_index]++], symbol);
                            changed = 1;
                        }
                        allCanBeEpsilon = 0;
                        break; // Stop processing further symbols.
                    } else {
                        // symbol is a non-terminal.
                        int sym_index = getindexofnonterminal(g, symbol);
                        // Add FIRST(symbol) except epsilon to FIRST(lhs)
                        for (t = 0; t < first_count[sym_index]; t++) {
                            if (strcmp(first_sets[sym_index][t], "ε") == 0)
                                continue;
                            int exists = 0;
                            for (int u = 0; u < first_count[lhs_index]; u++) {
                                if (strcmp(first_sets[lhs_index][u], first_sets[sym_index][t]) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                            if (!exists) {
                                strcpy(first_sets[lhs_index][first_count[lhs_index]++], first_sets[sym_index][t]);
                                changed = 1;
                            }
                        }
                        // Check if FIRST(symbol) contains epsilon.
                        int hasEpsilon = 0;
                        for (t = 0; t < first_count[sym_index]; t++) {
                            if (strcmp(first_sets[sym_index][t], "ε") == 0) {
                                hasEpsilon = 1;
                                break;
                            }
                        }
                        if (!hasEpsilon) {
                            allCanBeEpsilon = 0;
                            break;
                        }
                    }
                }
                // If all symbols in the alternative can derive ε, add ε to FIRST(lhs).
                if (allCanBeEpsilon) {
                    int exists = 0;
                    for (t = 0; t < first_count[lhs_index]; t++) {
                        if (strcmp(first_sets[lhs_index][t], "ε") == 0) {
                            exists = 1;
                            break;
                        }
                    }
                    if (!exists) {
                        strcpy(first_sets[lhs_index][first_count[lhs_index]++], "ε");
                        changed = 1;
                    }
                }
            }
        }
    }
}

void calculatefollowsets(Grammar g, char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH],int first_count[MAXIMUMSYMBOLS],char follow_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH],int follow_count[MAXIMUMSYMBOLS]) 
{
    int i, j, k, t, u, v;
   
    // Initialize FOLLOW sets for all non-terminals to empty.
    for (i = 0; i < g.nonterminalcount; i++) {
        follow_count[i] = 0;
    }
   
    // Add '$' to FOLLOW of the start symbol.
    int startIndex = getindexofnonterminal(g, g.startsymbol);
    if (startIndex != -1) {
        strcpy(follow_sets[startIndex][follow_count[startIndex]++], "$");
    }
   
    int changed = 1;
    while (changed) {
        changed = 0;
        // For every production A -> X1 X2 ... Xn.
        for (i = 0; i < g.productioncount; i++) {
            Production p = g.productions[i];
            int A_index = getindexofnonterminal(g, p.lhs);
            if (A_index == -1) continue;
            // For each alternative of the production.
            for (j = 0; j < p.rhscount; j++) {
                // For each symbol X in the alternative.
                for (k = 0; k < p.rhssymbols[j]; k++) {
                    char *X = p.rhs[j][k];
                    int X_index = getindexofnonterminal(g, X);
                    if (X_index == -1) continue; // X is terminal, so skip.
                   
                    // Process the tail: symbols after X in the alternative.
                    int tail_can_be_epsilon = 1; // Assume tail derives ε until proven otherwise.
                    for (t = k + 1; t < p.rhssymbols[j]; t++) {
                        char *Y = p.rhs[j][t];
                        // Check if Y is terminal or non-terminal.
                        if (getindexofnonterminal(g, Y) == -1) {
                            // Y is terminal; add Y to FOLLOW(X) if not already present.
                            int exists = 0;
                            for (u = 0; u < follow_count[X_index]; u++) {
                                if (strcmp(follow_sets[X_index][u], Y) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                            if (!exists) {
                                strcpy(follow_sets[X_index][follow_count[X_index]++], Y);
                                changed = 1;
                            }
                            tail_can_be_epsilon = 0; // Terminal cannot produce ε.
                            break;  // Stop processing further symbols in tail.
                        } else {
                            // Y is non-terminal.
                            int Y_index = getindexofnonterminal(g, Y);
                            // Add FIRST(Y) (excluding ε) to FOLLOW(X).
                            for (u = 0; u < first_count[Y_index]; u++) {
                                if (strcmp(first_sets[Y_index][u], "ε") == 0)
                                    continue;
                                int exists = 0;
                                for (v = 0; v < follow_count[X_index]; v++) {
                                    if (strcmp(follow_sets[X_index][v], first_sets[Y_index][u]) == 0) {
                                        exists = 1;
                                        break;
                                    }
                                }
                                if (!exists) {
                                    strcpy(follow_sets[X_index][follow_count[X_index]++], first_sets[Y_index][u]);
                                    changed = 1;
                                }
                            }
                            // Check if FIRST(Y) contains ε.
                            int Y_has_epsilon = 0;
                            for (u = 0; u < first_count[Y_index]; u++) {
                                if (strcmp(first_sets[Y_index][u], "ε") == 0) {
                                    Y_has_epsilon = 1;
                                    break;
                                }
                            }
                            if (!Y_has_epsilon) {
                                tail_can_be_epsilon = 0;
                                break; // Stop processing tail.
                            }
                        }
                    } // End processing tail.
                   
                    // If the tail (or no tail) can derive ε, add FOLLOW(A) to FOLLOW(X).
                    if (tail_can_be_epsilon) {
                        for (u = 0; u < follow_count[A_index]; u++) {
                            int exists = 0;
                            for (v = 0; v < follow_count[X_index]; v++) {
                                if (strcmp(follow_sets[X_index][v], follow_sets[A_index][u]) == 0) {
                                    exists = 1;
                                    break;
                                }
                            }
                            if (!exists) {
                                strcpy(follow_sets[X_index][follow_count[X_index]++], follow_sets[A_index][u]);
                                changed = 1;
                            }
                        }
                    }
                }
            }
        }
    }
}

// We encode a table entry as: entry = prodIndex * 1000 + altIndex
// (Assuming prodIndex and altIndex are less than 1000.)

void generateparsingtable(Grammar g,char first_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH],int first_count[MAXIMUMSYMBOLS],char follow_sets[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH],int follow_count[MAXIMUMSYMBOLS],int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS])
{
    // Initialize table cells to -1 (empty).
    for (int i = 0; i < g.nonterminalcount; i++) {
        for (int j = 0; j < g.terminalcount + 1; j++) { // +1 for '$'
            parsing_table[i][j] = -1;
        }
    }

    // Process each production.
    for (int prodIndex = 0; prodIndex < g.productioncount; prodIndex++) {
        int nt_index = getindexofnonterminal(g, g.productions[prodIndex].lhs);
        if (nt_index == -1) continue;

        // For each alternative in this production.
        for (int altIndex = 0; altIndex < g.productions[prodIndex].rhscount; altIndex++) {
            // If this alternative is exactly epsilon.
            if (g.productions[prodIndex].rhssymbols[altIndex] == 1 &&
                strcmp(g.productions[prodIndex].rhs[altIndex][0], "ε") == 0)
            {
                // For each terminal in FOLLOW(LHS), place this production.
                for (int f = 0; f < follow_count[nt_index]; f++) {
                    int col = getindexofterminal(g, follow_sets[nt_index][f]);
                    if (col == -1 && strcmp(follow_sets[nt_index][f], "$") == 0)
                        col = g.terminalcount;
                    if (col == -1)
                        continue;
                    if (parsing_table[nt_index][col] != -1) {
                        printf("Conflict in parsing table at [%s, %s]\n",
                               g.nonterminals[nt_index],
                               (col == g.terminalcount) ? "$" : g.terminals[col]);
                        fprintf(out, "Conflict in parsing table at [%s, %s]\n",
                               g.nonterminals[nt_index],
                               (col == g.terminalcount) ? "$" : g.terminals[col]);
                        printf("Grammar is not LL(1)!\n");
                        fprintf(out, "Grammar is not LL(1)!\n");
                    }
                    // Encode prodIndex and altIndex.
                    parsing_table[nt_index][col] = prodIndex * 1000 + altIndex;
                }
            }
            else {
                // Compute FIRST for this alternative.
                char first_of_alt[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH];
                int count_first = 0;
                int allNullable = 1;  // assume all symbols derive epsilon
                int n = g.productions[prodIndex].rhssymbols[altIndex];
                for (int s = 0; s < n; s++) {
                    char *sym = g.productions[prodIndex].rhs[altIndex][s];
                    int sym_nt_index = getindexofnonterminal(g, sym);
                    if (sym_nt_index == -1) {
                        // terminal: add it and stop.
                        addtoset(first_of_alt, &count_first, sym);
                        allNullable = 0;
                        break;
                    } else {
                        // non-terminal: add its FIRST (except epsilon).
                        for (int f = 0; f < first_count[sym_nt_index]; f++) {
                            if (strcmp(first_sets[sym_nt_index][f], "ε") != 0)
                                addtoset(first_of_alt, &count_first, first_sets[sym_nt_index][f]);
                        }
                        if (!checkepsilon(first_sets[sym_nt_index], first_count[sym_nt_index])) {
                            allNullable = 0;
                            break;
                        }
                    }
                }
                if (allNullable)
                    addtoset(first_of_alt, &count_first, "ε");

                // For every terminal in FIRST (except epsilon) fill table.
                int hasEpsilon = 0;
                for (int f = 0; f < count_first; f++) {
                    if (strcmp(first_of_alt[f], "ε") == 0) {
                        hasEpsilon = 1;
                        continue;
                    }
                    int col = getindexofterminal(g, first_of_alt[f]);
                    if (col == -1 && strcmp(first_of_alt[f], "$") == 0)
                        col = g.terminalcount;
                    if (col == -1)
                        continue;
                    if (parsing_table[nt_index][col] != -1) {
                        printf("Conflict in parsing table at [%s, %s]\n",
                               g.nonterminals[nt_index],
                               (col == g.terminalcount) ? "$" : g.terminals[col]);
                        printf("Grammar is not LL(1)!\n");
                    }
                    parsing_table[nt_index][col] = prodIndex * 1000 + altIndex;
                }
                // If epsilon is in FIRST, then for every terminal in FOLLOW(LHS) fill table.
                if (hasEpsilon) {
                    for (int f = 0; f < follow_count[nt_index]; f++) {
                        int col = getindexofterminal(g, follow_sets[nt_index][f]);
                        if (col == -1 && strcmp(follow_sets[nt_index][f], "$") == 0)
                            col = g.terminalcount;
                        if (col == -1)
                            continue;
                        if (parsing_table[nt_index][col] != -1) {
                            printf("Conflict in parsing table at [%s, %s]\n",
                                   g.nonterminals[nt_index],
                                   (col == g.terminalcount) ? "$" : g.terminals[col]);
                            printf("Grammar is not LL(1)!\n");
                        }
                        parsing_table[nt_index][col] = prodIndex * 1000 + altIndex;
                    }
                }
            }
        }
    }
}

void printgrammar(Grammar g) 
{
    for (int i = 0; i < g.productioncount; i++) {
        printf("%s -> ", g.productions[i].lhs);
        fprintf(out, "%s -> ", g.productions[i].lhs);
        for (int j = 0; j < g.productions[i].rhscount; j++) {
            for (int k = 0; k < g.productions[i].rhssymbols[j]; k++) {
                printf("%s ", g.productions[i].rhs[j][k]);
                fprintf(out, "%s ", g.productions[i].rhs[j][k]);
            }
           
            if (j < g.productions[i].rhscount - 1) {
                printf("| ");
                fprintf(out, "| ");
            }
        }
        printf("\n");
        fprintf(out, "\n");
    }
}

void printparsingtable(Grammar g, int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS]) 
{
    int totalCols = g.terminalcount + 1; // columns for each terminal plus '$'
    // Print header
    printf("%15s", "");
    fprintf(out, "%15s", "");
    for (int j = 0; j < g.terminalcount; j++) {
        printf("|%15s", g.terminals[j]);
     	fprintf(out, "|%15s", g.terminals[j]);
    }
    printf("|%15s\n", "$");
    fprintf(out, "|%15s\n", "$");
    for (int j = 0; j < totalCols; j++) {
        printf("+---------------");
        fprintf(out, "+---------------");
    }
    printf("+\n");
    fprintf(out, "+\n");

    // Print rows for each non-terminal.
    for (int i = 0; i < g.nonterminalcount; i++) {
        printf("%15s", g.nonterminals[i]);
        fprintf(out, "%15s", g.nonterminals[i]);
        for (int j = 0; j < totalCols; j++) {
            printf("|");
            fprintf(out, "|");
            if (parsing_table[i][j] != -1) {
                int code = parsing_table[i][j];
                int prodIndex = code / 1000;
                int altIndex = code % 1000;
                Production prod = g.productions[prodIndex];
                char prodStr[256] = "";
                sprintf(prodStr, "%s -> ", prod.lhs);

                // Print the alternative indicated by altIndex.
                for (int k = 0; k < prod.rhssymbols[altIndex]; k++) {
                    strcat(prodStr, prod.rhs[altIndex][k]);
                    if (k < prod.rhssymbols[altIndex] - 1)
                        strcat(prodStr, " ");
                }
                printf("%15s", prodStr);
                fprintf(out, "%15s", prodStr);
            } else {
                printf("%15s", "");
                fprintf(out, "%15s", "");
            }
        }
        printf("|\n");
        fprintf(out, "|\n");
        for (int j = 0; j < totalCols; j++) {
            printf("+---------------");
            fprintf(out, "+---------------");
        }
        printf("+\n");
        fprintf(out, "+\n");
    }
}


int checknonterminal(char* symbol) 
{
    return isupper(symbol[0]);
}

int getindexofnonterminal(Grammar g, char* symbol) 
{
    for (int i = 0; i < g.nonterminalcount; i++) {
        if (strcmp(g.nonterminals[i], symbol) == 0) {
            return i;
        }
    }
    return -1;
}

int getindexofterminal(Grammar g, char* symbol) 
{
    for (int i = 0; i < g.terminalcount; i++) {
        if (strcmp(g.terminals[i], symbol) == 0) {
            return i;
        }
    }
    return -1;
}

int checkepsilon(char set[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int count) 
{
    for (int i = 0; i < count; i++) {
        if (strcmp(set[i], "ε") == 0) {
            return 1;
        }
    }
    return 0;
}

void addtoset(char set[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH], int* count, char* symbol) 
{
    for (int i = 0; i < *count; i++) {
        if (strcmp(set[i], symbol) == 0) {
            return;
        }
    }
    strcpy(set[*count], symbol);
    (*count)++;
}


// Stack functions for Assignment 3

void initializestack(ParsingStack* stack) 
{
    stack->top = -1;
}

void push(ParsingStack* stack, char* symbol) 
{
    if (stack->top >= MAXIMUMSTACKSIZE - 1) {
        printf("Stack overflow\n");
        return;
    }
    strcpy(stack->items[++stack->top], symbol);
}

void pop(ParsingStack* stack) 
{
    if (stack->top < 0) {
        printf("Stack underflow\n");
        return;
    }
    stack->top--;
}

char* top(ParsingStack* stack) 
{
    if (stack->top < 0) {
        return NULL;
    }
    return stack->items[stack->top];
}

int isempty(ParsingStack* stack) 
{
    return stack->top < 0;
}

void printstack(ParsingStack* stack, FILE* file) 
{
    fprintf(file, "Stack: ");
    for (int i = 0; i <= stack->top; i++) {
        fprintf(file, "%s ", stack->items[i]);
    }
}

// Parse a single string using the LL(1) parsing table

int parsestring(Grammar g, int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS], char* input, int lineNo, FILE* outputFile) 
{
    ParsingStack stack;
    initializestack(&stack);
    
    // Initialize the stack with start symbol and end marker
    push(&stack, "$");
    push(&stack, g.startsymbol);
    
    // Save a copy of the original input for logging
    char originalInput[MAXIMUMINPUTLENGTH];
    strcpy(originalInput, input);
    
    // Break input into tokens
    char tokens[MAXIMUMSYMBOLS][MAXIMUMSYMBOLLENGTH];
    int token_count = 0;
    
    // Use a copy of input for tokenization to preserve the original
    char inputCopy[MAXIMUMINPUTLENGTH];
    strcpy(inputCopy, input);
    
    char* token = strtok(inputCopy, " \t\n");
    while (token != NULL && token_count < MAXIMUMSYMBOLS) {
        strcpy(tokens[token_count++], token);
        token = strtok(NULL, " \t\n");
    }
    
    // Add end marker to tokens
    strcpy(tokens[token_count++], "$");
    
    int tokenIndex = 0;
    int errorCount = 0;
    
    // Open a parsing log file
    FILE* logFile = fopen("parsing_log.txt", "a");
    if (!logFile) {
        printf("Failed to open parsing log file\n");
        return -1;
    }
    
    fprintf(logFile, "=== Parsing Line %d: \"%s\" ===\n", lineNo, originalInput);
    fprintf(outputFile, "=== Parsing Line %d: \"%s\" ===\n", lineNo, originalInput);
    printf("=== Parsing Line %d: \"%s\" ===\n", lineNo, originalInput);
    
    while (!isempty(&stack) && tokenIndex < token_count) {
        char* X = top(&stack);
        char* a = tokens[tokenIndex];
        
        printstack(&stack, logFile);
        fprintf(logFile, "\tInput: %s\n", a);
        
        // If X is a terminal or $
        if (getindexofnonterminal(g, X) == -1) {
            if (strcmp(X, a) == 0) { // Match
                fprintf(logFile, "Action: Match %s\n", X);
                pop(&stack);
                tokenIndex++;
            } else { // Error: terminal on stack doesn't match input
                fprintf(logFile, "Error: Expected %s, found %s\n", X, a);
                fprintf(outputFile, "Line %d: Syntax Error - Expected '%s', found '%s'\n", lineNo, X, a);
                printf("Line %d: Syntax Error - Expected '%s', found '%s'\n", lineNo, X, a);
                errorCount++;
                
                // Error recovery: pop the terminal from stack
                pop(&stack);
            }
        } else { // X is a non-terminal
            int row = getindexofnonterminal(g, X);
            int col = getindexofterminal(g, a);
            if (col == -1 && strcmp(a, "$") == 0) {
                col = g.terminalcount; // $ is at position after the last terminal
            }
            
            if (row != -1 && col != -1 && parsing_table[row][col] != -1) {
                // Parse table entry exists
                int code = parsing_table[row][col];
                int prodIndex = code / 1000;
                int altIndex = code % 1000;
                
                // Get the production and alternative
                Production prod = g.productions[prodIndex];
                
                fprintf(logFile, "Action: Expand %s -> ", X);
                for (int i = 0; i < prod.rhssymbols[altIndex]; i++) {
                    fprintf(logFile, "%s ", prod.rhs[altIndex][i]);
                }
                fprintf(logFile, "\n");
                
                // Pop the non-terminal
                pop(&stack);
                
                // Push the alternative in reverse order
                if (!(prod.rhssymbols[altIndex] == 1 && strcmp(prod.rhs[altIndex][0], "ε") == 0)) {
                    for (int i = prod.rhssymbols[altIndex] - 1; i >= 0; i--) {
                        push(&stack, prod.rhs[altIndex][i]);
                    }
                }
            } else { // Error: No entry in parsing table
                fprintf(logFile, "Error: No production for %s with input %s\n", X, a);
                fprintf(outputFile, "Line %d: Syntax Error - Unexpected symbol '%s'\n", lineNo, a);
                printf("Line %d: Syntax Error - Unexpected symbol '%s'\n", lineNo, a);
                errorCount++;
                
                // Error recovery: skip the input symbol
                tokenIndex++;
            }
        }
    }
    
    // Check if we consumed all input or have empty stack prematurely
    if (tokenIndex < token_count - 1) { // -1 because we added $ at the end
        fprintf(logFile, "Error: Extra input after parsing completed\n");
        fprintf(outputFile, "Line %d: Syntax Error - Extra input after parsing completed\n", lineNo);
        printf("Line %d: Syntax Error - Extra input after parsing completed\n", lineNo);
        errorCount++;
    }
    
    if (!isempty(&stack) && tokenIndex >= token_count) {
        fprintf(logFile, "Error: Input ended but stack is not empty\n");
        fprintf(outputFile, "Line %d: Syntax Error - Unexpected end of input\n", lineNo);
        printf("Line %d: Syntax Error - Unexpected end of input\n", lineNo);
        errorCount++;
    }
    
    if (errorCount == 0) {
        fprintf(logFile, "Parsing completed successfully\n");
        fprintf(outputFile, "Line %d: Parsing completed successfully\n", lineNo);
        printf("Line %d: Parsing completed successfully\n", lineNo);
    } else {
        fprintf(logFile, "Parsing completed with %d errors\n", errorCount);
        fprintf(outputFile, "Line %d: Parsing completed with %d errors\n", lineNo, errorCount);
        printf("Line %d: Parsing completed with %d errors\n", lineNo, errorCount);
    }
    
    fprintf(logFile, "\n");
    fclose(logFile);
    
    return errorCount;
}

// Process an entire input file with multiple lines to parse

void parseinput(Grammar g, int parsing_table[MAXIMUMSYMBOLS][MAXIMUMSYMBOLS], const char* inputfilename) 
{
    FILE* inputFile = fopen(inputfilename, "r");
    if (!inputFile) 
    {
        printf("Error opening input file: %s\n", inputfilename);
        return;
    }
    
    FILE* resultFile = fopen("parsing_results.txt", "w");
    if (!resultFile) 
    {
        printf("Failed to open parsing results file\n");
        fclose(inputFile);
        return;
    }
    
    // Clear the parsing log file
    FILE* logFile = fopen("parsing_log.txt", "w");
    if (logFile) 
    {
        fclose(logFile);
    }
    
    char line[MAXIMUMINPUTLENGTH];
    int lineNo = 1;
    int totalErrors = 0;
    
    fprintf(resultFile, "Parsing Results for Input File: %s\n\n", inputfilename);
    printf("Parsing Input File: %s\n\n", inputfilename);
    
    while (fgets(line, sizeof(line), inputFile)) 
    {
        // Remove newline
        line[strcspn(line, "\n")] = '\0';
        
        // Skip empty lines
        if (strlen(line) == 0) 
        {
            lineNo++;
            continue;
        }
        
        int errors = parsestring(g, parsing_table, line, lineNo, resultFile);
        if (errors > 0) 
        {
            totalErrors += errors;
        }
        
        lineNo++;
    }
    
    fprintf(resultFile, "\nParsing completed with a total of %d errors.\n", totalErrors);
    printf("\nParsing completed with a total of %d errors.\n", totalErrors);
    
    fclose(inputFile);
    fclose(resultFile);
}
