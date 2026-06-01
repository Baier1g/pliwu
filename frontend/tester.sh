#!/usr/bin/env bash

set -eo pipefail

TEST_ROOT="tests"

if [[ -n "$1" ]]; then
    TEST_ROOT=$(find . -type d -name "$1")
fi

FAIL=0

while read -rd '' file; do
    expected="${file%.oc}.true"

    if [[ ! -f "$expected" ]]; then
        #echo "Missing expected file: $expected"
        #FAIL=1
        continue
    fi
    echo "OC Testing $file"
    
    (
    ./grammar.out "$file" > /dev/null 2>&1
	nasm -f elf64 gen_asm.asm -o out.o && ld -m elf_x86_64 out.o -o out
    )

    if ! diff -q "$expected" <(./out); then
        FAIL=1
    fi

done < <(find "$TEST_ROOT" -name '*.oc' -print0)

if [[ $FAIL -eq 0 ]]; then
    echo "All tests passed"
else
    echo "Some test(s) failed"
fi

exit $FAIL