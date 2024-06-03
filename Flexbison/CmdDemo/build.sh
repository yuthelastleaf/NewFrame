rm -f parser.tab.h
rm -f parser.tab.c
rm -f lex.yy.c
bison -d parser.y
flex lexer.l
g++ -g -o cmddemo lex.yy.c parser.tab.c handle_param.cpp -L/c/msys64/usr/lib -lfl