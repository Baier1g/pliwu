CC = gcc
OUT = grammar.out
WFLAGS = -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-int-conversion
MEMFLAG = -fsanitize=address,leak,undefined

BISON = lex.yy.c y.tab.c
UTILS = linked_list.c hashmap.c symbol_table.c
SEMANTIC = ast.c scope.c type_checking.c
IR = codegen.c ir.c optimiser.c register_allocation.c ir_codegen.c

FILES = frontend.c $(BISON) $(UTILS) $(SEMANTIC) $(IR)

all:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)
	./grammar.out tests/test2.txt
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out

%:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)
	./grammar.out tests/$*.oc
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out

mem.%:
	flex rules.l
	bison -vdy grammar.y
	$(CC) $(WFLAGS) $(MEMFLAG) -o $(OUT) $(FILES)
	./grammar.out tests/$*.oc

flex.%:
	flex rules.l
	bison -vdy grammar.y
	y.output tests/$*.oc

.PHONY: clean
clean:
	rm -f out *.o *.out *.tab.c y.output grammar.out lex.yy.c gen_asm.asm