#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ast.h"
#include "schema.h"
#include "csv_gen.h"

/* External declarations */
extern int yyparse();
extern FILE *yyin;
extern JsonValue *json_root;

/* Command-line parsing */
void parse_arguments(int argc, char *argv[], int *print_ast, char **out_dir);

/* Main function */
int main(int argc, char *argv[]) {
    int print_ast = 0;
    char *out_dir = NULL;
    
    /* Parse command-line arguments */
    parse_arguments(argc, argv, &print_ast, &out_dir);
    
    /* Debug message */
    fprintf(stderr, "DEBUG: Starting JSON parsing from stdin\n");
    
    /* Parse JSON from stdin */
    yyin = stdin;
    
    /* Perform parsing */
    fprintf(stderr, "DEBUG: Starting parser\n");
    if (yyparse() != 0) {
        /* Parser error - already reported */
        fprintf(stderr, "DEBUG: Parser returned with error\n");
        return 1;
    }
    fprintf(stderr, "DEBUG: Parsing completed successfully\n");
    
    if (!json_root) {
        fprintf(stderr, "Error: No JSON data parsed\n");
        return 1;
    }
    
    /* Create schema context */
    SchemaContext *schema = create_schema_context(out_dir, print_ast);
    
    /* Detect schema from the JSON data */
    detect_schema(schema, json_root);
    
    /* Generate CSV files */
    generate_csv_files(schema, json_root);
    
    /* Clean up */
    free_schema_context(schema);
    free_json_value(json_root);
    if (out_dir) {
        free(out_dir);
    }
    
    return 0;
}

/* Parse command-line arguments */
void parse_arguments(int argc, char *argv[], int *print_ast, char **out_dir) {
    /* Default values */
    *print_ast = 0;
    *out_dir = NULL;
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--print-ast") == 0) {
            *print_ast = 1;
        } else if (strcmp(argv[i], "--out-dir") == 0) {
            if (i + 1 < argc) {
                *out_dir = strdup(argv[i + 1]);
                if (!*out_dir) {
                    fprintf(stderr, "Memory allocation failed for output directory\n");
                    exit(1);
                }
                i++; /* Skip the next argument (directory name) */
            } else {
                fprintf(stderr, "Error: --out-dir requires a directory name\n");
                exit(1);
            }
        } else {
            fprintf(stderr, "Unknown argument: %s\n", argv[i]);
            fprintf(stderr, "Usage: %s [--print-ast] [--out-dir DIR]\n", argv[0]);
            exit(1);
        }
    }
}
