%{
#include <string.h>
#include <stdio.h> // 包含stdio.h以使用printf
    int chars = 0;
    int words = 0;
    int lines = 0;
%}
%%
[a-zA-Z]+   { words++; chars += strlen(yytext); }
\n          { chars++; lines++; }
.           { chars++; }
%%
int main(int argc, char **argv) {
    yylex();
    printf("%8d%8d%8d\n", lines, words, chars);
    return 0; // 返回一个值表示程序正常结束
}
