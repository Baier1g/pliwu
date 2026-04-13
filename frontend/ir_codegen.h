#ifndef IR_CODEGEN_H
#define IR_CODEGEN_H

#include "ast.h"
#include "ir.h"
#include "linked_list.h"
#include "register_allocation.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum CG_op_code CG_op_code;
typedef enum CG_operand_mode CG_operand_mode;
typedef enum CG_operand_type CG_operand_type;

typedef struct CG_operation CG_operation;
typedef struct CG_operand CG_operand;

enum CG_op_code {
    CG_MOV,
    CG_LEA,
    CG_PUSH,
    CG_POP,
    
    CG_ADD,
    CG_SUB,
    CG_MULT,
    CG_DIV,

    CG_CMP,
    CG_XOR,

    CG_JMP,
    CG_JNE,
    CG_JE,
    CG_JG,
    CG_JL,

    CG_CALL,
    CG_RET,
    CG_SYSCALL,
    CG_LABEL,

};

enum CG_operand_type {
    O_REGISTER,
    O_CONSTANT,
    O_LABEL,
    O_DISPLACEMENT,
};

/*
 * Enum for describing addressing modes.
 * None is for things like a label
 * Indirect is a dereference
 * Direct is the constant value or value in register
 */
enum CG_operand_mode {
    NONE,
    DIRECT,
    INDIRECT,
};

struct CG_operation {
    CG_op_code operation;
    CG_operand *operand1;
    CG_operand *operand2;
    char *comment;
};

struct CG_operand {
    CG_operand_type type;
    CG_operand_mode mode;
    union {
        reg_color reg;
        int constant;
        char *label;
        struct {
            reg_color reg;
            int offset;
        } displacement;
    };
};

/*
 * Creates a CG_operand with the provided type, mode and arguments.
 */
CG_operand *CG_create_operand(CG_operand_type, CG_operand_mode, void *, void *);

/*
 * Creates a CG_operation from the provided IR_operation
 */
CG_operation *CG_create_operation(IR_operation *);

/*
 * Main entrypoint for code generation
 */
void codegen(linked_list *, frame *, RA_graph *);

#endif