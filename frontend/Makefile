CC = gcc
OUT = grammar.out
WFLAGS = -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-int-conversion

all:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) -o $(OUT) lex.yy.c y.tab.c ast.c linked_list.c

.PHONY: clean
clean:
	rm *.tab.* y.output grammar.out lex.yy.c