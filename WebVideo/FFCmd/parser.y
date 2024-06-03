%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "handle_param.h"  // Include the header file

// Include necessary Flex declarations
extern int yylex();
extern int yyparse();
void yyerror(const char *s);
typedef struct yy_buffer_state* YY_BUFFER_STATE;
extern YY_BUFFER_STATE yy_scan_string(const char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE b);
extern char *yytext;
%}

%union {
    char* str;
}

%token PARAM_D PARAM_F PARAM_I
%token <str> STRING
%token ERROR

%%

input:
    /* empty */
    | input param
    | input error_param
    ;

param:
    PARAM_D STRING { handle_param_d($2); }
    | PARAM_F STRING { handle_param_f($2); }
    | PARAM_I STRING { handle_param_i($2); }
    ;

error_param:
    PARAM_D { yyerror("Error: -d parameter missing value"); }
    | PARAM_F { yyerror("Error: -f parameter missing value"); }
    | PARAM_D error { yyerror("Error: -d parameter followed by invalid value"); }
    | PARAM_F error { yyerror("Error: -f parameter followed by invalid value"); }
    ;

%%

void yyerror(const char *s) {
    fprintf(stderr, "%s\n", s);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s -d \"param\" -f \"param\"\n", argv[0]);
        return 1;
    }

    // Concatenate all arguments into a single string
    size_t length = 0;
    for (int i = 1; i < argc; ++i) {
        length += strlen(argv[i]) + 1;
    }
    char* input = (char*)malloc(length + 1);
    input[0] = '\0';
    for (int i = 1; i < argc; ++i) {
        strcat(input, argv[i]);
        strcat(input, " ");
    }

    printf("input is %s\n", input);

    // Use yy_scan_string to handle input
    YY_BUFFER_STATE buffer = yy_scan_string(input);
    yyparse();
    yy_delete_buffer(buffer);
    free(input);

    return 0;
}
