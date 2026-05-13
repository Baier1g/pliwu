CC = gcc
OUT = grammar.out
WFLAGS = -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast -Wno-int-conversion
MEMFLAG = -fsanitize=address,leak,undefined

BISON = lex.yy.c grammar.tab.c
UTILS = linked_list.c hashmap.c symbol_table.c
SEMANTIC = ast.c scope.c type_checking.c
IR = codegen.c ir.c optimiser.c register_allocation.c ir_codegen.c

FILES = frontend.c $(BISON) $(UTILS) $(SEMANTIC) $(IR)

all: flexbison
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)
	./grammar.out tests/test2.txt
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out

%: #can not have flexbison as dependency, as then clean doesn't work for some reason.
	make flexbison		
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)
	./grammar.out tests/$*.oc
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out

test.%:
	make flexbison		
	$(CC) $(WFLAGS) -o $(OUT) $(FILES)
	./grammar.out oc_tests/$*.oc
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out
	

nasm:
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out
	./out
nasm.%:
	nasm -f elf64 $*.asm -o out.o && ld -m elf_x86_64 out.o -o out
	./out

mem.%: flexbison
	$(CC) $(WFLAGS) $(MEMFLAG) -o $(OUT) $(FILES)
	./grammar.out tests/$*.oc

flex.%: flexbison
	$(CC) $(WFLAGS) -o flexer.out $(BISON) $(UTILS) $(SEMANTIC)
	./flexer.out tests/$*.oc

flexbison:
	flex rules.l
	bison -vd grammar.y

.PHONY: clean
clean:
	rm -f out *.o *.out *.tab.c y.output grammar.out lex.yy.c gen_asm.asm