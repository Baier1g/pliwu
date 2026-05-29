#!/usr/bin/env bash

set -euo pipefail

TEST_ROOT="testFails"

FAIL=0

echo " == Remember to turn off debug prints!"

while read -rd '' file; do
    expected="${file%.oc}.fail"

    if [[ ! -f "$expected" ]]; then
        #echo "Missing expected file: $expected"
        #FAIL=1
        continue
    fi

    echo "OC Testing $file"
    
    if ! diff -u "$expected" <(./grammar.out "$file"); then
        FAIL=1
    fi

done < <(find "$TEST_ROOT" -name '*.oc' -print0)

if [[ $FAIL -eq 0 ]]; then
    echo "All tests passed"
else
    echo "Some test(s) failed"
fi

exit $FAIL