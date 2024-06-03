%{
#include "parser.tab.h"
#include <cstring>
#include "handle_param.h"
%}

%option noyywrap

%%


-d  { printf("Matched -d\n"); return PARAM_D; }
-f  { printf("Matched -f\n"); return PARAM_F; }
[ \t\n]+  { /* ignore whitespace */ }
[^ ]+ { yylval.str = strdup(yytext); printf("Matched string: %s\n", yylval.str); return STRING; }

%%
