#ifndef IR_H
#define IR_H

#include "ast.h"
#include "linked_list.h"
#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct frame frame;
typedef struct segment segment;
typedef struct operation operation;
typedef struct operand operand;

typedef enum op_code op_code;
typedef enum operand_type operand_type;

char *op_code_to_string(op_code);
operation *create_op(op_code, operand *, operand *, operand *);
frame *create_frame();
frame *create_named_frame(char *);

segment *create_segment(symbol_table *);

/*
 * Creates an operand.
 * The operand_type defines which member of the union is instantiated.
 */
operand *create_operand(operand_type, void *);

frame *create_IR_tree(AST_node *);

/*
 * VERY RUDIMENTARY
 */
enum op_code
{
    /*MOV,
    LEA,
    PUSH,
    POP,*/
    IR_ASSIGN,
    IR_PARAM,

    IR_ADD,
    // ADC,
    IR_SUB,
    IR_MUL,
    IR_DIV,

    IR_EQUALS,
    IR_NEQUALS,
    IR_LESS,
    IR_GREATER,
    IR_LESS_EQ,
    IR_GREATER_EQ,

    IR_AND,
    IR_OR,

    /*CMP,
    XOR,*/

    IR_PRINT,
    IR_CALL,
    IR_RET,
    // INT,

    IR_GOTO,
    /*JNC,
    JMP,
    JNE,
    JE,
    JG,
    JL,
    JGE,
    JLE,*/
};

enum operand_type {
    P_CONSTANT,
    P_TEMP,
    P_LABEL,
    P_VARIABLE,
    P_FUNC_CALL,
};

/*
 * A struct representing a stack frame i.e. a function
 */
struct frame {
    char *name;
    segment *segment;
    linked_list *nested_frames;
};

struct segment {
    segment *left, *right;
    symbol_table *table;
    linked_list *operations;
};

/*
 * A struct representing an instruction
 */
struct operation {
    op_code op;
    operand *arg1;
    operand *arg2;
    operand *arg3;
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
        segment *dest;
        frame *call;
    };
};


#endif