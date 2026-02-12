all: parse

grammar.tab.c grammar.tab.h: grammar.y
	bison -t -d -v grammar.y

lex.yy.c: rules.l grammar.tab.h
	flex rules.l

parse: lex.yy.c grammar.tab.c grammar.tab.h
	gcc -o parse grammar.tab.c 

.PHONY: clean
clean:
	rm grammar.tab.* grammar.output lex.yy.c *.o