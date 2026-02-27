CC = gcc
OUT = grammar.out
WFLAGS = -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-int-conversion
MEMFLAG = -fsanitize=address,leak,undefined

FILES = lex.yy.c y.tab.c ast.c linked_list.c frontend.c hashmap.c symbol_table.c scope.c codegen.c

all:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)
	./grammar.out asm_test.txt
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out

mem:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) $(MEMFLAG) -o $(OUT) $(FILES)

.PHONY: clean
clean:
	rm *.tab.c y.output grammar.out lex.yy.c *.o