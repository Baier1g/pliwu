#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int indents = 0;

void print_indents(void);

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
        case A_PROGRAM:
            node->program.modules = a;
            break;
        case A_MODULE:
            node->module.module_declarations = a;
            break;
        case A_EXPR_STMT:
            node->expr_stmt.expression = a;
            break;
        case A_PRINT_STMT:
            node->print_stmt.expression = a;
            break;
        case A_BLOCK_STMT:
            node->block.stmt_list = a;
        case A_RETURN_STMT:
            node->return_stmt.expression = a;
            break;
        default:
            printf("Unexpected kind in create_unary_node: %d\n", node_kind);
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
                case TYPE_INT:
                    node->primary_expr.integer_value = (int) b;
                    break;
                case TYPE_CHAR:
                    node->primary_expr.char_value = (char) b;
                    break;
                case TYPE_BOOL:
                    node->primary_expr.bool_value = (short) b;
                    break;
                case TYPE_IDENTIFIER:
                    size_t len = strlen((char*) b);
                    node->primary_expr.identifier_name = malloc(sizeof(char) * len);
                    memset(node->primary_expr.identifier_name, 0, len);
                    strncpy(node->primary_expr.identifier_name, b, len);
                    break;
            }
            break;
        case A_UNARY_EXPR:
            node->unary_expr.op = (unary_op) a;
            node->unary_expr.expression = b;
            break;
        case A_ASSIGN_EXPR:
            node->assign_expr.identifier = a;
            node->assign_expr.expression = b;
            break;
        case A_PARAMETER_EXPR:
            node->parameter.type = (data_type) a;
            node->parameter.identifier = b;
            break;
        case A_CALL_EXPR:
            node->call_expr.identifier = a;
            node->call_expr.arguments = b;
            break;
        default:
            printf("Unexpected kind in create_binary_node: %d\n", node_kind);
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
            node->if_stmt.else_branch = c;
            break;
        case A_ARITHMETIC_EXPR:
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
            node->binary_expr.left = a;
            node->binary_expr.op = (binary_op) b;
            node->binary_expr.right = c;
            break;
        case A_VAR_DECL:
            node->var_decl.type = (data_type) a;
            node->var_decl.identifier = b;
            node->var_decl.expr_stmt = c;
            break;
        default:
            printf("Unexpected kind in create_ternary_node: %d\n", node_kind);
            break;
        }
    return node;
}

struct AST_node *create_quaternary_node(int startchar, int line, kind node_kind, void *a, void *b, void *c, void *d) {
    struct AST_node *node = malloc(sizeof(struct AST_node));
    if (!node) {
        return NULL;
    }
    node->pos = create_pos(startchar, line);
    node->kind = node_kind;
    switch (node_kind) {
        case A_FUNC_DEF:
            node->func_def.return_type = (data_type) a;
            node->func_def.identifier = b;
            node->func_def.parameters = c;
            node->func_def.function_block = d;
            break;
        default:
            printf("Unexpected kind in create_quaternary_node: %d\n", node_kind);
            break;
        }
    return node;
}

char *kind_enum_to_string(kind type) {
    char* kind = malloc(sizeof(char) * 30);
    switch(type) {
        case A_PROGRAM:
            strcpy(kind, "PROGRAM");
            break;
        case A_MODULE:
            strcpy(kind, "MODULE");
            break;
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
        case A_RELATIONAL_EXPR:
            strcpy(kind, "Relational expression");
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
        case A_PARAMETER_EXPR:
            strcpy(kind, "Parameter expression");
            break;
        default:
            strcpy(kind, "Unknown kind");
            break;
    }
    return kind;
}

char *binary_op_enum_to_string(binary_op operand) {
    char* op = malloc(sizeof(char) * 4);
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
        case A_AND:
            strcpy(op, "AND");
            break;
        case A_OR:
            strcpy(op, "OR");
            break;
    }
    return op;
}

void AST_printer(struct AST_node *node) {
    if (!node) {
        return;
    }
    char* c;
    print_indents();
    printf("%s:\n", (c = kind_enum_to_string(node->kind)));
    free(c);
    switch(node->kind) {
        case A_PROGRAM:
            {
                indents++;
                linked_list_node *ptr = node->program.modules->head;
                while (ptr) {
                    AST_printer(ptr->data);
                    ptr = ptr->next;
                }
                indents--;
            }
            break;
        case A_MODULE:
            {
                indents++;
                linked_list_node *ptr = node->module.module_declarations->head;
                while (ptr) {
                    AST_printer(ptr->data);
                    ptr = ptr->next;
                }
                indents--;
            }
            break;
        case A_FUNC_DEF:
            indents++;
            print_indents();
            printf("return type: %d\n", node->func_def.return_type);
            AST_printer(node->func_def.identifier);
            linked_list_node *ptrs = node->func_def.parameters->head;
            while (ptrs) {
                AST_printer(ptrs->data);
                ptrs = ptrs->next;
            }
            AST_printer(node->func_def.function_block);
            indents--;
            break;
        case A_PARAMETER_EXPR:
            indents++;
            print_indents();
            printf("type: %d\n", node->parameter.type);
            AST_printer(node->parameter.identifier);
            indents--;
            break;
        case A_VAR_DECL:
            indents++;
            print_indents();
            printf("type: %d\n", node->var_decl.type);
            AST_printer(node->var_decl.identifier);
            AST_printer(node->var_decl.expr_stmt);
            indents--;
            break;
        case A_BLOCK_STMT:
            linked_list_node *ptr = node->block.stmt_list->head;
            indents++;
            while (ptr) {
                AST_printer(ptr->data);
                ptr = ptr->next;
            }
            indents--;
            break;
        case A_IF_STMT:
            indents++;
            AST_printer(node->if_stmt.condition);
            AST_printer(node->if_stmt.if_branch);
            AST_printer(node->if_stmt.else_branch);
            indents--;
            break;
        case A_PRINT_STMT:
            indents++;
            AST_printer(node->print_stmt.expression);
            indents--;
            break;
        case A_EXPR_STMT:
            break;
        case A_RETURN_STMT:
            indents++;
            AST_printer(node->return_stmt.expression);
            indents--;
            break;
        case A_ASSIGN_EXPR:
            indents++;
            AST_printer(node->assign_expr.identifier);
            AST_printer(node->assign_expr.expression);
            indents--;
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            indents++;
            AST_printer(node->binary_expr.left);
            char* d;
            print_indents();
            printf("Operator: %s\n", (d = binary_op_enum_to_string(node->binary_expr.op)));
            free(d);
            AST_printer(node->binary_expr.right);
            indents--;
            break;
        case A_UNARY_EXPR:
            indents++;
            print_indents();
            printf("Operator: %c\n", node->unary_expr.op == A_NEG ? '!' : '-');
            AST_printer(node->unary_expr.expression);
            indents--;
            break;
        case A_PRIMARY_EXPR:
            indents++;
            print_indents();
            printf("Type: %d\n", node->primary_expr.type);
            print_indents();
            switch (node->primary_expr.type) {
                case TYPE_INT:
                    printf("Literal: %d\n", node->primary_expr.integer_value);
                    break;
                case TYPE_CHAR:
                    printf("Literal: %c\n", node->primary_expr.char_value);
                    break;
                case TYPE_BOOL:
                    printf("Literal: %s\n", node->primary_expr.bool_value ? "True" : "False");
                    break;
                case TYPE_IDENTIFIER:
                    printf("Literal: %s\n", node->primary_expr.identifier_name);
                    break;
                case TYPE_VOID:
                    printf("Literal: None\n");
                    break;
                default:
                    printf("Someting went wong UwU");
                    break;
            }
            indents--;
            break;
        case A_CALL_EXPR:
            indents++;
            AST_printer(node->call_expr.identifier);
            linked_list_node *llpeter = node->call_expr.arguments->head;
            while (llpeter) {
                AST_printer(llpeter->data);
                llpeter = llpeter->next;
            }
            indents--;
            break;
        default:
            break;
    }
}

void print_indents() {
    for (int i = 0; i < indents; i++) {
        printf("|   ");
    }
}

/*int main() {
    char *id = malloc(sizeof(char) * 10);
    strncpy(id, "hello", 10);
    struct AST_node *node = create_binary_node(10, 2, A_PRIMARY_EXPR, TYPE_INT,  (void *) 50);
    struct AST_node *unary_node = create_unary_node(10, 2, A_EXPR_STMT, node);
    struct AST_node *node_2 = create_binary_node(10, 2, A_PRIMARY_EXPR, (void *) TYPE_IDENTIFIER, id);
    struct AST_node *ternary_node = create_ternary_node(30, 10, A_ARITHMETIC_EXPR, node, (void *) A_ADD, node_2);

    char *c;
    printf("Digit in node: %d\n", node->primary_expr.integer_value);
    printf("Pos value in unary_node: %d, %d\n", unary_node->pos.line, unary_node->pos.startchar);
    printf("Value in nested nodes: %d\n", unary_node->expr_stmt.expression->primary_expr.integer_value);
    printf("Value in left child: %d, value in right child: %s, operation: %s\n", ternary_node->binary_expr.left->primary_expr.integer_value, ternary_node->binary_expr.right->primary_expr.identifier_name, (c = binary_op_enum_to_string(ternary_node->binary_expr.op)));

    free(c);
    free(id);
    free(unary_node->expr_stmt.expression);
    free(unary_node);
    free(ternary_node->binary_expr.right->primary_expr.identifier_name);
    free(ternary_node->binary_expr.right);
    free(ternary_node);
}*/