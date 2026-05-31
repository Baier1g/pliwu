#!/usr/bin/env bash
file=$(find . -type f -name "$1.oc" | head -n 1)

if [[ $file ]]; then
    ./grammar.out $file && nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out
else
    echo "error: $1.oc not found"
fi