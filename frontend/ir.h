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
typedef struct IR_operation IR_operation;
typedef struct IR_operand IR_operand;

typedef struct RA_graph RA_graph;
typedef struct RA_node RA_node;

typedef enum IR_op_code IR_op_code;
typedef enum operand_type operand_type;
typedef enum reg_color reg_color;

/*
 * Returns a string representing the op code provided
 */
char *IR_op_code_to_string(IR_op_code);

/*
 * Creates an operation from the provided arguments
 */
IR_operation *create_op(IR_op_code, IR_operand *, IR_operand *, IR_operand *);

void handle_use_set(IR_operation *);

void handle_def_set(IR_operation *);

/*
 * Creates an unnamed frame struct
 */
frame *create_frame();

/*
 * Creates named frame struct
 */
frame *create_named_frame(char *);

/*
 * Creates a segment struct with the provided symbol table
 */
segment *create_segment(symbol_table *);

/*
 * Creates an operand.
 * The operand_type defines which member of the union is instantiated.
 */
IR_operand *create_operand(operand_type, void *);

/*
 * The main entrypoint of intermediate code generation.
 * It takes the root of an AST as argument
 */
frame *create_IR_tree(AST_node *);

/*
 * Prints the IR tree to the terminal
 */
void print_IR_tree(frame *);

/*
 * Performs liveness analysis on the provided IR by populating the in- and out set of the each operation.
 */
void liveness(frame *);

/*
 * This enum describes the different types of operations in the intermediate representation
 */
enum IR_op_code {
    /*MOV,
    LEA,
    PUSH,
    POP,*/
    IR_VAR_DECL,
    IR_ASSIGN,
    IR_PARAM,
    IR_POP_PARAM,

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

    IR_IF,
    IR_WHILE,
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

/*
 * This enum holds different types of operand used in the intermediate representation
 */
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
    segment *segment, *last;
    linked_list *nested_frames;
};

/*
 * A struct representing a straight line segment of the code
 */
struct segment {
    int iteration;
    linked_list *pred;
    segment *left, *right;
    symbol_table *table;
    linked_list *operations;
};

/*
 * A struct representing an instruction
 */
struct IR_operation {
    linked_list *in, *out, *use, *def;
    enum IR_op_code op;
    IR_operand *arg1;
    IR_operand *arg2;
    IR_operand *arg3;
};

/*
 * A struct representing an operand.
 * Can be either a constant, a temporary or a label
 */
struct IR_operand {
    operand_type type;
    union {
        char *variable_name;
        int constant;
        segment *dest;
        frame *call;
    };
};

/*
 * A struct that represents a graph of temporary variables.
 */
struct RA_graph {
    int num_nodes;
    int **adj_matrix;
    RA_node **nodes;
};

/* A struct that represents a node in the register allocation graph 
 * Color holds the allocated register of a temporary
 * The connections array holds the indices of the temporaries that are alive at the same time
 */
struct RA_node {
    int color;
    int num_edges;
    int *connections;
};

enum reg_color {
    R15 = 1,
    R14,
    R13,
    R12,
    R11,
    R10,
    RBX,
    R9,
    R8,
    RCX,
    RAX,
    RDX,
};

RA_graph *create_graph(int);
RA_node *create_graph_node(int);
void connect_nodes(RA_graph *, int, int);
void connect_graph(RA_graph *, frame *);
void print_graph(RA_graph *);
void print_adj_matrix(RA_graph *);




#endif