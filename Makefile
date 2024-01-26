## Jared Kaiser
## Spring 2023
## Makefile

all: lab9

lab9: lab9.l lab9.y ast.h ast.c symtable.c symtable.h emit.h emit.c
	lex lab9.l
	yacc -d lab9.y
	gcc lex.yy.c y.tab.c symtable.c ast.c emit.c -o lab9

clean:
	  rm -f lab9
