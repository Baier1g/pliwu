#include "visitor.h"

/* Stolen valor */

void passthrough (struct AST_node* node){
    if (node->kind != A_PROGRAM || node->kind != A_MODULE){
        return;
    }
    accept_stmt(node);
}


void visit_program(struct AST_node* node) {
    printf("Program\n");

    linked_list_node *ptr = node->program.modules->head;
    while (ptr) {
        accept_stmt(ptr->data);
        ptr = ptr->next;
    }    
}
void visit_module (struct AST_node* node){
    printf("Module\n");

    linked_list_node *ptr = node->module.module_declarations->head;
    while (ptr) {
        accept_stmt(ptr->data);
        ptr = ptr->next;
    }
}
void visit_func_def(struct AST_node* node){
    printf("Function %s returns %d", 
        node->func_def.identifier->primary_expr.identifier_name,
        node->func_def.return_type);
        
    linked_list_node *ptrs = node->func_def.parameters->head;
    printf("(");
    if(ptrs){
        while (ptrs) {
            accept_expr(ptrs->data);
            ptrs = ptrs->next;
            if (ptrs->next) {
                printf(", ");
            }
        }
    }
    printf(")\n");
    accept_stmt(node->func_def.function_block);
}
void visit_var_decl(struct AST_node* node) {
    printf("Variable %s of type: %d\n", node->var_decl.identifier->primary_expr.identifier_name, 
                                        node->var_decl.type);
    if (node->var_decl.expr_stmt) {
        accept_expr(node->var_decl.expr_stmt);
    }
}

int alphabet = 97;
void visit_block(struct AST_node* node){
    printf("Block %c start\n", alphabet);
    alphabet++;

    linked_list_node *ptr = node->block.stmt_list->head;
    while (ptr) {
        accept_stmt(ptr->data);
        ptr = ptr->next;
    }

    alphabet--;
    printf("Block %c end\n", alphabet);
}
void visit_if(struct AST_node* node) {
    printf("If condition\n");
    accept_expr(node->if_stmt.condition);
    printf("Thenbranch\n");
    accept_stmt(node->if_stmt.if_branch);
    if(node->if_stmt.else_branch) {
        printf("Elsebranch\n");
    }
    accept_stmt(node->if_stmt.else_branch);
}
void visit_print(struct AST_node* node) {
    printf("Print\n");
    accept_expr(node->print_stmt.expression);
}
void visit_expr(struct AST_node* node){
    printf("Expression stmt\n");
    accept_expr(node);
}


void* visit_return(struct AST_node* node) {
    printf("return\n");
    accept_expr(node->return_stmt.expression);
}
void* visit_assign(struct AST_node* node) {
    printf("assigning %s with\n", node->assign_expr.identifier->primary_expr.identifier_name);
    accept_expr(node->assign_expr.expression);
}
void* visit_logical(struct AST_node* node) {
    printf("logical\n");
    accept_expr(node->binary_expr.left);
    printf("operator %d\n", node->binary_expr.op);
    accept_expr(node->binary_expr.right);
}
void* visit_relational(struct AST_node* node) {
    printf("relational\n");
    accept_expr(node->binary_expr.left);
    printf("operator %d\n", node->binary_expr.op);
    accept_expr(node->binary_expr.right);
}
void* visit_arithmetic(struct AST_node* node) {
    printf("arithmetic\n");
    accept_expr(node->binary_expr.left);
    printf("operator %d\n", node->binary_expr.op);
    accept_expr(node->binary_expr.right);
}
void* visit_unary(struct AST_node* node) {
    printf("unary with operator: %c\n", node->unary_expr.op == A_NEG ? '!' : '-');
    accept_expr(node->unary_expr.expression);
}
void* visit_primary(struct AST_node* node) {
    printf("Primary with type: %d Literal: ", node->primary_expr.type);
    switch (node->primary_expr.type) {
        case TYPE_INT:
            printf("%d\n", node->primary_expr.integer_value);
            break;
        case TYPE_CHAR:
            printf("%c\n", node->primary_expr.char_value);
            break;
        case TYPE_BOOL:
            printf("%s\n", node->primary_expr.bool_value ? "True" : "False");
            break;
        case TYPE_IDENTIFIER:
            printf("(name) %s\n", node->primary_expr.identifier_name);
            break;
        case TYPE_VOID:
            printf("None (void)\n");
            break;
        default:
            printf("Literal type not supported");
    }
}
void* visit_call(struct AST_node* node) {
    printf("call to %s with args:\n", node->call_expr.identifier->primary_expr.identifier_name);
    {   linked_list_node *ptr = node->call_expr.arguments->head;
        while (ptr) {
            accept_expr(ptr->data);
            ptr = ptr->next;
        }
    }
}
void* visit_parameter(struct AST_node* node) {
    printf("t%d %s", node->parameter.type,
            node->parameter.identifier->primary_expr.identifier_name);
}
