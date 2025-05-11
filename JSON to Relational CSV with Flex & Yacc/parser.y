%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Enable debugging */
#define DEBUG_PARSER 1

/* Parser state */
extern int yylex();
extern int line;
extern int column;
extern FILE *yyin;

/* Root node of the AST */
JsonValue *json_root = NULL;

/* Error handling */
void yyerror(const char *s);

/* Debug printing function */
void debug_print(const char *msg) {
    if (DEBUG_PARSER) {
        fprintf(stderr, "PARSER: %s\n", msg);
    }
}
%}

/* Define value types */
%union {
    double dval;
    char *sval;
    int bval;
    JsonValue *json_val;
    KeyValuePair *kv_pair;
    KeyValuePair *kv_list;
    ArrayElement *arr_elem;
    ArrayElement *arr_list;
}

/* Define tokens */
%token <dval> NUMBER
%token <sval> STRING
%token <bval> TRUE FALSE
%token NUL
%token LBRACE RBRACE LBRACKET RBRACKET COLON COMMA

/* Define non-terminal types */
%type <json_val> json_value
%type <json_val> json_object
%type <json_val> json_array
%type <kv_pair> json_pair
%type <kv_list> json_pairs
%type <arr_elem> json_element
%type <arr_list> json_elements

/* Enable location tracking for error messages */
%locations

%%

json:
    json_value  { 
        debug_print("Completed parsing JSON value");
        json_root = $1; 
    }
    ;

json_value:
    json_object       { 
        debug_print("Parsed object");
        $$ = $1; 
    }
    | json_array      { 
        debug_print("Parsed array");
        $$ = $1; 
    }
    | STRING          { 
        debug_print("Parsed string");
        $$ = create_string($1, @1.first_line, @1.first_column); free($1); 
    }
    | NUMBER          { 
        debug_print("Parsed number");
        $$ = create_number($1, @1.first_line, @1.first_column); 
    }
    | TRUE            { 
        debug_print("Parsed true");
        $$ = create_boolean(1, @1.first_line, @1.first_column); 
    }
    | FALSE           { 
        debug_print("Parsed false");
        $$ = create_boolean(0, @1.first_line, @1.first_column); 
    }
    | NUL             { 
        debug_print("Parsed null");
        $$ = create_null(@1.first_line, @1.first_column); 
    }
    ;

json_object:
    LBRACE RBRACE                 { 
        debug_print("Parsed empty object {}");
        $$ = create_object(@1.first_line, @1.first_column); 
    }
    | LBRACE json_pairs RBRACE    { 
        debug_print("Parsed object with key-value pairs");
        $$ = create_object(@1.first_line, @1.first_column);
                                     
        /* Add all key-value pairs to the object */
        KeyValuePair *pair = $2;
        while (pair) {
            KeyValuePair *next = pair->next;
            pair->next = NULL;
            add_key_value($$, pair->key, pair->value);
            if (DEBUG_PARSER) {
                fprintf(stderr, "PARSER: Added key '%s' to object\n", pair->key);
            }
            free(pair->key);
            free(pair);
            pair = next;
        }
    }
    ;

json_pairs:
    json_pair                     { 
        debug_print("Parsed first key-value pair");
        $$ = $1; 
    }
    | json_pairs COMMA json_pair  { 
        debug_print("Parsed additional key-value pair");
        /* Add new pair to the end of the list */
        if ($1) {
            KeyValuePair *current = $1;
            while (current->next) {
                current = current->next;
            }
            current->next = $3;
            $$ = $1;
        } else {
            $$ = $3;
        }
    }
    ;

json_pair:
    STRING COLON json_value       { 
        if (DEBUG_PARSER) {
            fprintf(stderr, "PARSER: Created key-value pair for key '%s'\n", $1);
        }
        $$ = (KeyValuePair*)malloc(sizeof(KeyValuePair));
        if (!$$) {
            yyerror("Memory allocation failed");
            exit(1);
        }
        $$->key = $1;
        $$->value = $3;
        $$->next = NULL;
    }
    ;

json_array:
    LBRACKET RBRACKET             { 
        debug_print("Parsed empty array []");
        $$ = create_array(@1.first_line, @1.first_column); 
    }
    | LBRACKET json_elements RBRACKET {
        debug_print("Parsed array with elements");
        $$ = create_array(@1.first_line, @1.first_column);
                                     
        /* Add all elements to the array */
        ArrayElement *elem = $2;
        while (elem) {
            ArrayElement *next = elem->next;
            elem->next = NULL;
            add_array_element($$, elem->value);
            free(elem);
            elem = next;
        }
    }
    ;

json_elements:
    json_element                      { 
        debug_print("Parsed first array element");
        $$ = $1; 
    }
    | json_elements COMMA json_element {
        debug_print("Parsed additional array element");
        /* Add new element to the end of the list */
        if ($1) {
            ArrayElement *current = $1;
            while (current->next) {
                current = current->next;
            }
            current->next = $3;
            $$ = $1;
        } else {
            $$ = $3;
        }
    }
    ;

json_element:
    json_value                     {
        debug_print("Created array element");
        $$ = (ArrayElement*)malloc(sizeof(ArrayElement));
        if (!$$) {
            yyerror("Memory allocation failed");
            exit(1);
        }
        $$->value = $1;
        $$->next = NULL;
    }
    ;

%%

/* Error handling */
void yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d, column %d\n", s, yylloc.first_line, yylloc.first_column);
    
    /* Print additional debug info */
    if (DEBUG_PARSER) {
        fprintf(stderr, "DEBUG: Last error occurred at line %d, column %d\n", 
                line, column);
        fprintf(stderr, "DEBUG: Check for issues in the JSON around this position\n");
        
        /* Print context information if available */
        extern char *yytext;
        if (yytext) {
            fprintf(stderr, "DEBUG: Last token text: '%s'\n", yytext);
        }
    }
}
