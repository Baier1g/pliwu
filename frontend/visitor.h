#ifndef VISITOR
#define VISITOR 

#include <stdio.h>
#include "ast.h"

void visit_program(struct AST_node*);
void visit_module (struct AST_node*);
void visit_func_def(struct AST_node*);
void visit_var_decl(struct AST_node*);
void visit_block(struct AST_node*);
void visit_if(struct AST_node*);
void visit_print(struct AST_node*);
void visit_expr(struct AST_node*);

void* visit_return(struct AST_node*);
void* visit_assign(struct AST_node*);
void* visit_logical(struct AST_node*);
void* visit_relational(struct AST_node*);
void* visit_arithmetic(struct AST_node*);
void* visit_unary(struct AST_node*);
void* visit_primary(struct AST_node*);
void* visit_call(struct AST_node*);
void* visit_parameter(struct AST_node*);


void accept_stmt(struct AST_node* node) {
    if (!node){return;}
    
    switch(node->kind) {
        case A_PROGRAM:
            visit_program(node);
            break;
        case A_MODULE:
            visit_module(node);
            break;
        case A_FUNC_DEF:
            visit_func_def(node);
            break;
        case A_VAR_DECL:
            visit_var_decl(node);
            break;
        case A_BLOCK_STMT:
            visit_block(node);
            break;
        case A_IF_STMT:
            visit_if(node);
            break;
        case A_PRINT_STMT:
            visit_print(node);
            break;
        case A_EXPR_STMT:
            visit_expr(node);
            break;
        default:
            printf("Error: accept_stmt called on unknown kind");
    }
}

void* accept_expr(struct AST_node* node) {
    switch (node->kind){
        case A_RETURN_STMT:
            return visit_return(node);
        case A_ASSIGN_EXPR:
            return visit_assign(node);
        case A_LOGICAL_EXPR:
            return visit_logical(node);
        case A_RELATIONAL_EXPR:
            return visit_relational(node);
        case A_ARITHMETIC_EXPR:
            return visit_arithmetic(node);
        case A_UNARY_EXPR:
            return visit_unary(node);
        case A_PRIMARY_EXPR:
            return visit_primary(node);
        case A_CALL_EXPR:
            return visit_call(node);
        case A_PARAMETER_EXPR:
            return visit_parameter(node);
        default:
            printf("Error: accept_expr called on unknown kind");
        return NULL;
    }
}

#endif