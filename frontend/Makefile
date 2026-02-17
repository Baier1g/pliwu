CC = gcc
OUT = grammar.out

all:
	flex rules.l
	bison -vdy grammar.y
	$(CC) -o $(OUT) lex.yy.c y.tab.c ast.c

.PHONY: clean
clean:
	rm *.tab.* y.output grammar.out lex.yy.c