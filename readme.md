# OC Language Compiler

A quickstart guide for using OC

This guide assumes the user is using linux, and has gcc, flex, bison, 
and nasm as well as the command line functions find, read, grep, and diff

It is recommended to use the "userprograms" folder for programs, and name
them in the scheme "OC_<name>.oc". The .oc part of the filename is required.

## How to run

enter the "frontend" folder. From here, the following make rules can be used to run programs

make <filename>
- Scans all folders (incl. frontend) for a <filename>.oc file to run.
    - Do not write .oc in the makerule.
- Be careful not to name the file the same as one of the tests

make tester
- Runs all tests in the "tests" folder and reports if any don't output 
  the same as what is written in the .true file next to the oc file.

make testerFail
- Runs all tests in the testFails folder and reports if any don't output
  the same as what is written in the .fail file next to the oc file.

make tester.<folder>
- Runs all tests in the given folder and reports if any don't output 
  the same as what is written in the .true file next to the oc file.

make old.<filename>
- Tries to compile with the old codegen. Not recommended and not supported.
