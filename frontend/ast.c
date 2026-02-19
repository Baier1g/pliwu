#include "ast.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int indents = 0;

void print_indents(void);
void kill_ll(linked_list*);

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
                    printf("%ld length\n", len);
                    node->primary_expr.identifier_name = calloc(len + 1, sizeof(char));
                    //memset(node->primary_expr.identifier_name, 0, len + 1);
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
    switch(type) {
        case A_PROGRAM:
            return "PROGRAM";
        case A_MODULE:
            return "MODULE";
        case A_FUNC_DEF:
            return "Function definition";
        case A_VAR_DECL:
            return "Variable declaration";
        case A_BLOCK_STMT:
            return "Block statement";
        case A_IF_STMT:
            return "If statement";
        case A_PRINT_STMT:
            return "Print statement";
        case A_EXPR_STMT:
            return "Expression statement";
        case A_RETURN_STMT:
            return "Return statement";
        case A_ASSIGN_EXPR:
            return "Assignment expression";
        case A_LOGICAL_EXPR:
            return "Logical expression";
        case A_RELATIONAL_EXPR:
            return "Relational expression";
        case A_ARITHMETIC_EXPR:
            return "Arithmetic expression";
        case A_UNARY_EXPR:
            return "Unary expression";
        case A_PRIMARY_EXPR:
            return "Primary expression";
        case A_CALL_EXPR:
            return "Call expression";
        case A_PARAMETER_EXPR:
            return "Parameter expression";
        default:
            return "Unknown kind";
    }
}

char *binary_op_enum_to_string(binary_op operand) {
    switch(operand) {
        case A_LESS:
            return "<";
        case A_GREATER:
            return ">";
        case A_LESS_EQ:
            return "<=";
        case A_GREATER_EQ:
            return ">=";
        case A_EQUALS:
            return "==";
        case A_NEQUALS:
            return "!=";
        case A_ASSIGN:
            return "=";
        case A_ADD:
            return "+";
        case A_SUB:
            return "-";
        case A_MULT:
            return "*";
        case A_DIV:
            return "/";
        case A_AND:
            return "AND";
        case A_OR:
            return "OR";
        default:
            return "Unkown operator";
    }
}

void kill_tree(struct AST_node* node) {
    if (!node) {
        return;
    }
    kind type = node->kind;
    linked_list_node *tmp;
    switch (type) {
        case A_PROGRAM:
            kill_ll(node->program.modules);
            free(node);
            break;
        case A_MODULE:
            kill_ll(node->module.module_declarations);
            free(node);
            break;
        case A_FUNC_DEF:
            kill_tree(node->func_def.identifier);
            kill_ll(node->func_def.parameters);
            kill_tree(node->func_def.function_block);
            free(node);
            break;
        case A_VAR_DECL:
            kill_tree(node->var_decl.identifier);
            kill_tree(node->var_decl.expr_stmt);
            free(node);
            break;
        case A_BLOCK_STMT:
            kill_ll(node->block.stmt_list);
            free(node);
            break;
        case A_IF_STMT:
            kill_tree(node->if_stmt.condition);
            kill_tree(node->if_stmt.if_branch);
            kill_tree(node->if_stmt.else_branch);
            free(node);
            break;
        case A_PRINT_STMT:
            kill_tree(node->print_stmt.expression);
            free(node);
            break;
        case A_EXPR_STMT:
            kill_tree(node->expr_stmt.expression);
            free(node);
            break;
        case A_RETURN_STMT:
            kill_tree(node->return_stmt.expression);
            free(node);
            break;
        case A_ASSIGN_EXPR:
            kill_tree(node->assign_expr.identifier);
            kill_tree(node->assign_expr.expression);
            free(node);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            kill_tree(node->binary_expr.left);
            kill_tree(node->binary_expr.right);
            free(node);
            break;
        case A_UNARY_EXPR:
            kill_tree(node->unary_expr.expression);
            free(node);
            break;
        case A_PRIMARY_EXPR:
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                free(node->primary_expr.identifier_name);
            }
            free(node);
            break;
        case A_CALL_EXPR:
            kill_tree(node->call_expr.identifier);
            kill_ll(node->call_expr.arguments);
            free(node);
            break;
        case A_PARAMETER_EXPR:
            kill_tree(node->parameter.identifier);
            free(node);
            break;
        default:
            printf("This should not happen :thinking:");
            break;
    }
    return;
}

void AST_printer(struct AST_node *node) {
    if (!node) {
        return;
    }
    print_indents();
    printf("%s:\n", kind_enum_to_string(node->kind));
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
            print_indents();
            printf("Operator: %s\n", binary_op_enum_to_string(node->binary_expr.op));
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

void kill_ll(linked_list *ll) {
    linked_list_node *curr, *next;
    curr = ll->head;
    while (curr) {
        next = curr->next;
        kill_tree(curr->data);
        free(curr);
        curr = next;
    }
    free(ll);
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