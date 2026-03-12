#ifndef IR_H
#define IR_H

#include "ast.h"
#include "linked_list.h"
#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct operation operation;
typedef enum op_code op_code;
typedef struct operand operand;
typedef enum operand_type operand_type;

char *op_code_to_string(op_code);
operation create_op(operand *, operand *, char *);

/*
 * VERY RUDIMENTARY
 */
enum op_code {
    MOV,
    LEA,
    PUSH,
    POP,

    ADD,
    ADC,
    SUB,
    IMUL,
    DIV,
    
    CMP,
    XOR,

    CALL,
    RET,
    JNC,
    JMP,
    JNE,
    JE,
    JG,
    JL,
    JGE,
    JLE,
};

enum operand_type {
    CONSTANT,
    TEMP,
    LABEL,
};

struct operation {
    op_code op;
    operand *arg1;
    operand *arg2;
    char* comment;
};

struct operand {
    operand_type type;
    union {
        int constant;
        operation *dest;
    };
};


#endif