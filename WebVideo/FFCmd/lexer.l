%{
#include "parser.tab.h"
#include <string.h>
#include "handle_param.h"
%}

%option noyywrap

%%

-i  { printf("Matched -i\n"); return PARAM_I;}
-d  { printf("Matched -d\n"); return PARAM_D; }
-f  { printf("Matched -f\n"); return PARAM_F; }
[ \t\n]+  { /* ignore whitespace */ }
\"[^\"]+\" { yylval.str = strdup(yytext+1); yytext[yyleng-1] = 0; printf("Matched string: %s\n", yylval.str); return STRING; }
[^ ]+ { yylval.str = strdup(yytext); printf("Matched string: %s\n", yylval.str); return STRING; }

%%