#ifndef IR_H
#define IR_H

#include "ast.h"
#include "linked_list.h"
#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct frame frame;
typedef struct block block;
typedef struct operation operation;
typedef struct operand operand;

typedef enum op_code op_code;
typedef enum operand_type operand_type;

char *op_code_to_string(op_code);
operation *create_op(op_code, operand *, operand *, char *);
frame *create_frame();

block *create_block

/*
 * Sets the second argument as the next property of the first operation argument
 */
void link_operations(operation *, operation *);

/*
 * Creates an operand.
 * The operand_type defines which member of the union is instantiated.
 */
operand *create_operand(operand_type, void *);

operation *create_IR_tree(AST_node *);

/*
 * VERY RUDIMENTARY
 */
enum op_code {
    MOV,
    LEA,
    PUSH,
    POP,
    PARAM,

    ADD,
    ADC,
    SUB,
    IMUL,
    DIV,
    
    CMP,
    XOR,

    LABEL,
    CALL,
    RET,
    INT,

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
    VARIABLE,
};

/*
 * A struct representing a stack frame i.e. a function
 */
struct frame {
    char *name;
    operation* operations;
    frame **nested_frames;
};

struct block {
    block *left, *right;
    symbol_table *table;
    operation *start;
};

/*
 * A struct representing an instruction
 */
struct operation {
    operation *next, *prev;
    op_code op;
    operand *arg1;
    operand *arg2;
    operand *arg3;
    char* comment;
};

/*
 * A struct representing an operand.
 * Can be either a constant, a temporary or a label
 */
struct operand {
    operand_type type;
    union {
        char *variable_name;
        int constant;
        operation *dest;
        frame *call;
    };
};


#endif