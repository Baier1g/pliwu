#ifndef AST_H
#define AST_H

#include "linked_list.h"

typedef enum {
    A_PROGRAM,
    A_MODULE,

    A_FUNC_DEF, // return_type, parameter*, function_block
    A_VAR_DECL, // type, identifier, expression+

    A_BLOCK_STMT,  // declaration*
    A_IF_STMT,     // condition, if_block, else_block
    A_PRINT_STMT,  // expression
    A_EXPR_STMT,   // expression
    A_RETURN_STMT, // type, expression

    A_ASSIGN_EXPR,     // identifier, expression
    A_LOGICAL_EXPR,    // expression, operator, expression
    A_PARAMETER_EXPR,
    A_RELATIONAL_EXPR,   // expression, operator, expression
    A_ARITHMETIC_EXPR, // expression, operator, expression
    A_UNARY_EXPR,      // operator, expression
    A_PRIMARY_EXPR,    // type, literal
    A_CALL_EXPR,       // identifier, argument*
} kind;

typedef enum {
    A_AND,
    A_OR,
    A_LESS,
    A_GREATER,
    A_LESS_EQ,
    A_GREATER_EQ,
    A_EQUALS,
    A_NEQUALS,
    A_ASSIGN,
    A_ADD,
    A_SUB,
    A_MULT,
    A_DIV,
} binary_op;

typedef enum {
    A_NEG,
    A_MINUS,
} unary_op;

typedef enum {
    TYPE_INT,
    TYPE_CHAR,
    TYPE_BOOL,
    TYPE_IDENTIFIER,
    TYPE_VOID
} data_type;

typedef struct {
    int startchar;
    int line;
} pos;

/*
 * Creates a unary AST node.
 * int startchar: The starting character of this node.
 * int line: The line this node is found on.
 * kind node_kind: The type of AST_node to be created.
 * void *a: The content of this node.
 *
 * The kinds of unary node:
 * Expression Statement
 * Return statement (a: *AST_node)
 * Print Statement
 * Block Statement
 * Module
 * Program
 */
struct AST_node *create_unary_node(int, int, kind, void *);

/*
 * Creates a binary AST node.
 * int startchar: The starting character of this node.
 * int line: The line this node is found on.
 * kind node_kind: The type of AST_node to be created.
 * void *a: The first content of this node.
 * void *b: The second content of this node.
 * 
 * The kinds of binary node:
 * Primary expression (a: data_type, b: literal value)
 * Unary expression (a: unary_op, b: *AST_node)
 * Assignment expression (a: *AST_node identifier, b: *AST_node)
 * Parameter expression (a: data_type type, b: AST_node* identifier)
 */
struct AST_node *create_binary_node(int, int, kind, void *, void *);

/*
 * Creates a ternary AST node.
 * int startchar: The starting character of this node.
 * int line: The line this node is found on.
 * kind node_kind: The type of AST_node to be created.
 * void *a: The first content of this node.
 * void *b: The second content of this node.
 * void *c: The third content of this node.
 *
 * The kinds of ternary node:
 * If statement (a: condition, b: thenbranch, c: (optional) elsebranch)
 * Arithmetic expression (a: left operand, b: operator, c: right operand)
 * Logical expression (a: left operand, b: operator, c: right operand)
 * Relational expression (a: left operand, b: operator, c: right operand)
 * Variable declaration (a : datatype, b: identifier, c: (optional) expression)
 */
struct AST_node *create_ternary_node(int, int, kind, void *, void *, void *);

struct AST_node *create_quaternary_node(int, int, kind, void *, void *, void *, void *);

/*
 * Recursively frees all memory associated with the node
 */
void kill_tree(struct AST_node*);

/*
 * Recursively prints the abstract syntax tree
 */
void AST_printer(struct AST_node*);

// AST_node
struct AST_node {
    kind kind;
    pos pos;
    union {
        // Program node
        struct {
            linked_list *modules;
        } program;

        // Module node
        struct {
            linked_list *module_declarations;
        } module;

        // Function definition
        struct {
            data_type return_type;
            struct AST_node *identifier;
            linked_list *parameters;
            struct AST_node *function_block;
        } func_def;

        // Variable declaration
        struct {
            data_type type;
            struct AST_node *identifier;
            struct AST_node *expr_stmt;
        } var_decl;

        // Block statement
        struct {
            linked_list *stmt_list;
        } block;

        // If statement
        struct {
            struct AST_node *condition;
            struct AST_node *if_branch;
            struct AST_node *else_branch;
        } if_stmt;

        // Print statement
        struct {
            struct AST_node *expression;
        } print_stmt;

        // Expression statement
        struct {
            struct AST_node *expression;
        } expr_stmt;

        // Return statement
        struct {
            struct AST_node *expression;
        } return_stmt;

        // Assignment expression
        struct {
            struct AST_node *identifier;
            struct AST_node *expression;
        } assign_expr;

        // Binary expression
        struct {
            struct AST_node *left;
            binary_op op;
            struct AST_node *right;
        } binary_expr;

        // Unary expression
        struct {
            unary_op op;
            struct AST_node *expression;
        } unary_expr;

        // Call expression
        struct {
            struct AST_node *identifier;
            struct linked_list *arguments;
        } call_expr;

        // Primary expression
        struct {
            data_type type;
            union {
                int integer_value;
                char char_value;
                short bool_value;
                char* identifier_name;
            };
        } primary_expr;

        // Parameter expression
        struct {
            data_type type;
            struct AST_node *identifier;
        } parameter;
    };
};

#endif