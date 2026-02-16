#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

pos create_pos(int start, int line) {
    pos pos;
    pos.startchar = start;
    pos.line = line;
    return pos;
}

struct AST_node *create_unary_node(int startchar, int line, kind node_kind, void *a) {
    struct AST_node *node = malloc(sizeof(struct AST_node));
    if (!node) {
        return NULL;
    }
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    switch (node_kind) {
        case A_EXPR_STMT:
            node->expr_stmt.expression = a;
            break;
        case A_PRINT_STMT:
            node->print_stmt.expression = a;
            break;
    }
    return node;
}

struct AST_node *create_binary_node(int startchar, int line, kind node_kind, void *a, void *b) {
    struct AST_node *node = malloc(sizeof(struct AST_node));
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    switch (node_kind) {
        case A_PRIMARY_EXPR:
            node->primary_expr.type = (data_type) a;
            switch (node->primary_expr.type) {
                case T_INT:
                    node->primary_expr.integer_value = (int) b;
                    break;
                case T_CHAR:
                    node->primary_expr.char_value = (char) b;
                    break;
                case T_BOOL:
                    node->primary_expr.bool_value = (short) b;
                    break;
            }
            break;
        case A_RETURN_STMT:
            node->return_stmt.return_type = (data_type) a;
            node->return_stmt.expression = b;
            break;
    }
    return node;
}

struct AST_node *create_ternary_node(int startchar, int line, kind node_kind, void* a, void* b, void *c) {
    struct AST_node *node = malloc(sizeof(struct AST_node));
    if (!node) {
        return NULL;
    }
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    switch (node_kind) {
        case A_IF_STMT:
            node->if_stmt.condition = a;
            node->if_stmt.if_branch = b;
            node->if_stmt.if_branch = c;
            break;
        case A_ARITHMETIC_EXPR:
        case A_LOGICAL_EXPR:
        case A_EQUALITY_EXPR:
            node->binary_expr.left = a;
            node->binary_expr.op = (binary_op) b;
            node->binary_expr.right = c;
            break;
    }
    return node;
}

char *kind_enum_to_string(kind type) {
    char* kind = malloc(sizeof(char) * 30);
    switch(type) {
        case A_FUNC_DEF:
            strcpy(kind, "Function definition");
            break;
        case A_VAR_DECL:
            strcpy(kind, "Variable declaration");
            break;
        case A_BLOCK_STMT:
            strcpy(kind, "Block statement");
            break;
        case A_IF_STMT:
            strcpy(kind, "If statement");
            break;
        case A_PRINT_STMT:
            strcpy(kind, "Print statement");
            break;
        case A_EXPR_STMT:
            strcpy(kind, "Expression statement");
            break;
        case A_RETURN_STMT:
            strcpy(kind, "Return statement");
            break;
        case A_ASSIGN_EXPR:
            strcpy(kind, "Assignment expression");
            break;
        case A_LOGICAL_EXPR:
            strcpy(kind, "Logical expression");
            break;
        case A_EQUALITY_EXPR:
            strcpy(kind, "Equality expression");
            break;
        case A_ARITHMETIC_EXPR:
            strcpy(kind, "Arithmetic expression");
            break;
        case A_UNARY_EXPR:
            strcpy(kind, "Unary expression");
            break;
        case A_PRIMARY_EXPR:
            strcpy(kind, "Primary expression");
            break;
        case A_CALL_EXPR:
            strcpy(kind, "Call expression");
            break;
    }
    return kind;
}

char *binary_op_enum_to_string(binary_op operand) {
    char* op = malloc(sizeof(char) * 3);
    switch(operand) {
        case A_LESS:
            strcpy(op, "<");
            break;
        case A_GREATER:
            strcpy(op, ">");
            break;
        case A_LESS_EQ:
            strcpy(op, "<=");
            break;
        case A_GREATER_EQ:
            strcpy(op, ">=");
            break;
        case A_EQUALS:
            strcpy(op, "==");
            break;
        case A_NEQUALS:
            strcpy(op, "!=");
            break;
        case A_ASSIGN:
            strcpy(op, "=");
            break;
        case A_ADD:
            strcpy(op, "+");
            break;
        case A_SUB:
            strcpy(op, "-");
            break;
        case A_MULT:
            strcpy(op, "*");
            break;
        case A_DIV:
            strcpy(op, "/");
            break;
    }
    return op;
}

int main() {
    struct AST_node *node = create_binary_node(10, 2, A_PRIMARY_EXPR, T_INT,  (void *) 50);
    struct AST_node *unary_node = create_unary_node(10, 2, A_EXPR_STMT, node);
    struct AST_node *node_2 = create_binary_node(10, 2, A_PRIMARY_EXPR, T_INT, (void *) 100);
    struct AST_node *ternary_node = create_ternary_node(30, 10, A_ARITHMETIC_EXPR, node, (void *) A_ADD, node_2);

    char *c;
    printf("Digit in node: %d\n", node->primary_expr.integer_value);
    printf("Pos value in unary_node: %d, %d\n", unary_node->pos.line, unary_node->pos.startchar);
    printf("Value in nested nodes: %d\n", unary_node->expr_stmt.expression->primary_expr.integer_value);
    printf("Value in left child: %d, value in right child: %d, operation: %s\n", ternary_node->binary_expr.left->primary_expr.integer_value, ternary_node->binary_expr.right->primary_expr.integer_value, (c = binary_op_enum_to_string(ternary_node->binary_expr.op)));

    free(c);
    free(unary_node->expr_stmt.expression);
    free(unary_node);
    free(ternary_node->binary_expr.right);
    free(ternary_node);
}