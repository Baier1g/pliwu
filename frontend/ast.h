#ifndef AST_H
#define AST_H

typedef enum {
    A_FUNC_DEF, // return_type, parameter*, function_block
    A_VAR_DECL, // type, identifier, expression+

    A_BLOCK_STMT,  // declaration*
    A_IF_STMT,     // condition, if_block, else_block
    A_PRINT_STMT,  // expression
    A_EXPR_STMT,   // expression
    A_RETURN_STMT, // type, expression

    A_ASSIGN_EXPR,     // identifier, expression
    A_LOGICAL_EXPR,    // expression, operator, expression
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

struct linked_list {
    struct linked_list_node *head;
    struct linked_list_node *tail;
};

struct linked_list_node {
    struct linked_list_node *next;
    void *content;
};

struct linked_list *create_linked_list(void *);
struct linked_list_node *create_node(void *);
void add_node(struct linked_list *, struct linked_list *);


struct AST_node *create_unary_node(int, int, kind, void *);
struct AST_node *create_binary_node(int, int, kind, void *, void *);
struct AST_node *create_ternary_node(int, int, kind, void *, void *, void *);
void AST_printer(struct AST_node*);

struct AST_node {
    kind kind;
    pos pos;
    union {
        // Function definition
        struct {
            data_type return_type;
            struct AST_node **parameters;
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
            int num_nodes;
            struct AST_node *expr_stmt;
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
            data_type return_type;
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
            struct AST_node **arguments;
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
    };
};

#endif