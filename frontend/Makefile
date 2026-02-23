CC = gcc
OUT = grammar.out
WFLAGS = -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-int-conversion
MEMFLAG = -fsanitize=address,leak,undefined

FILES = lex.yy.c y.tab.c ast.c linked_list.c frontend.c hashmap.c symbol_table.c scope.c

all:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)

mem:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) $(MEMFLAG) -o $(OUT) $(FILES)

.PHONY: clean
clean:
	rm *.tab.c y.output grammar.out lex.yy.c