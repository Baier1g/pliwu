#ifndef CODE_GEN_H
#define CODE_GEN_H

#include "ast.h"
#include "linked_list.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct operation* operation;
typedef enum op_code op_code;
typedef struct operand* operand;

char *op_code_to_string(op_code);
void generate_code(linked_list *, struct AST_node *);
void generate_code_helper(struct AST_node *);

/*
 * VERY RUDIMENTARY
 */
enum op_code {
    MOV,
    PUSH,
    POP,

    ADD,
    ADC,
    SUB,
    MUL,
    DIV,
    
    CMP,
    XOR,

    JMP,
    JNE,
    JE,
    JG,
    JL
};

struct operation {
    op_code op;
    operand arg1;
    operand arg2;
};

struct operand {
    union {
        char* reg;
        int constant;
    };
};


#endif