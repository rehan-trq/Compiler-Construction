%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "parser.tab.h"

/* Track line and column numbers */
int line = 1;
int column = 1;

/* Update column counter */
#define YY_USER_ACTION { \
    yylloc.first_line = line; \
    yylloc.first_column = column; \
    fprintf(stderr, "Scanner: Line %d, Col %d, Token '%s'\n", line, column, yytext); \
    column += yyleng; \
}
%}

%option noyywrap
%option noinput
%option nounput

%%

[ \t]           { /* ignore but count column */ }
\n              { line++; column = 1; }
\r\n            { line++; column = 1; }
\r              { line++; column = 1; }

"{"             { fprintf(stderr, "TOKEN: LBRACE\n"); return LBRACE; }
"}"             { fprintf(stderr, "TOKEN: RBRACE\n"); return RBRACE; }
"["             { fprintf(stderr, "TOKEN: LBRACKET\n"); return LBRACKET; }
"]"             { fprintf(stderr, "TOKEN: RBRACKET\n"); return RBRACKET; }
":"             { fprintf(stderr, "TOKEN: COLON\n"); return COLON; }
","             { fprintf(stderr, "TOKEN: COMMA\n"); return COMMA; }

"true"          { fprintf(stderr, "TOKEN: TRUE\n"); yylval.bval = 1; return TRUE; }
"false"         { fprintf(stderr, "TOKEN: FALSE\n"); yylval.bval = 0; return FALSE; }
"null"          { fprintf(stderr, "TOKEN: NULL\n"); return NUL; }

[0-9]+\.[0-9]+([eE][+-]?[0-9]+)?  { 
                    fprintf(stderr, "TOKEN: NUMBER (FLOAT) %s\n", yytext);
                    yylval.dval = atof(yytext); 
                    return NUMBER; 
                }
-?[0-9]+([eE][+-]?[0-9]+)?  { 
                    fprintf(stderr, "TOKEN: NUMBER (INT) %s\n", yytext);
                    yylval.dval = atof(yytext); 
                    return NUMBER; 
                }

\"[^\"]*\"      { 
                    /* Simple string handling - no escape sequences */
                    fprintf(stderr, "TOKEN: STRING %s\n", yytext);
                    /* Remove quotes */
                    int len = strlen(yytext);
                    char *str = (char*)malloc(len - 1);
                    if (!str) {
                        fprintf(stderr, "Memory allocation failed\n");
                        exit(1);
                    }
                    strncpy(str, yytext + 1, len - 2);
                    str[len - 2] = '\0';
                    yylval.sval = str;
                    return STRING;
                }

.               { 
                    fprintf(stderr, "Error: Invalid character '%c' (ASCII: %d) at line %d, column %d\n",
                            yytext[0], (int)yytext[0], line, column - 1);
                    exit(1);
                }

%%
