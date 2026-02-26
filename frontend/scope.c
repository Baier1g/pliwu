#include "scope.h"
#include "symbol_table.h"

#define MAX_ARGUMENTS 64

linked_list *scope_errors;
symbol_table *current_scope; // string name -> ast_node
linked_list *calls;          // list->data = ast_node
short is_in_function = 0;

void to_error(char *error_msg, struct AST_node *n) {
    //find line #, find startchar with helper
    linked_list_append(scope_errors, error_msg);
}

void recurse_scope(struct AST_node *node) {
    symbol_table *outer_table;
    switch(node->kind) {
        case A_PROGRAM:
            printf("error -> scope: entered with kind A_PROGRAM");
            break;
        case A_MODULE:
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_scope((struct AST_node *) lln->data);
            }
            break;
        case A_FUNC_DEF:
            // [x] check if scope is global scope
            // [x] check param maxlength
            // [x] check name
            // [x] name -> symboltable
            if (current_scope != (current_scope->global)){
                to_error("function definition not in global scope", node);
                return;
            }
            if (node->func_def.parameters->size > MAX_ARGUMENTS){
                to_error("funciton definition has too many parameters", node);
                return;
            }
            if (symbol_table_insert(current_scope,node->func_def.identifier->primary_expr.identifier_name, node)){
                to_error("function name already used", node);
                return;
            }
            
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            is_in_function = 1;

            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            recurse_scope(node->func_def.function_block);
            
            destroy_symbol_table(current_scope);
            current_scope = outer_table;
            is_in_function = 0;
            break;
        case A_VAR_DECL:
            if (node->var_decl.expr_stmt) {
                recurse_scope(node->var_decl.expr_stmt);
            }
            if(symbol_table_insert(current_scope, node->var_decl.identifier->primary_expr.identifier_name, node)) {
                to_error("Variable name already in use (in this or global scope)", node);
            }
            break;
        case A_BLOCK_STMT:
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        case A_IF_STMT:
            recurse_scope(node->if_stmt.condition);

            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            recurse_scope(node->if_stmt.if_branch);

            destroy_symbol_table(current_scope);
            current_scope = create_symbol_table(outer_table, outer_table->global);
            if (node->if_stmt.else_branch) {
                recurse_scope(node->if_stmt.else_branch);
            }
            
            destroy_symbol_table(current_scope);
            current_scope = outer_table;
            break;
        case A_PRINT_STMT:
            recurse_scope(node->print_stmt.expression);
            break;
        case A_EXPR_STMT:
            printf("error -> scope: entered with kind A_EXPR_STMT");
            break;
        case A_RETURN_STMT:
            if (!is_in_function) {
                to_error("returned outside of function", node);
            }
            break;
        case A_ASSIGN_EXPR:
            recurse_scope(node->assign_expr.identifier);
            recurse_scope(node->assign_expr.expression);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            recurse_scope(node->binary_expr.left);
            recurse_scope(node->binary_expr.right);
            break;
        case A_UNARY_EXPR:
            recurse_scope(node->unary_expr.expression);
            break;
        case A_PRIMARY_EXPR:
            if (node->primary_expr.type == TYPE_IDENTIFIER){
                if (!symbol_table_contains(current_scope, node->primary_expr.identifier_name)){
                    to_error("Undefined identifier", node);
                    break;
                }
                struct AST_node *a = (struct AST_node *) symbol_table_get(current_scope, node->primary_expr.identifier_name);
                if (a->kind == A_FUNC_DEF) {
                    to_error("Tried to access a function when variable was expected.", node);
                    break;
                }
            }
            break;
        case A_CALL_EXPR:
            linked_list_append(calls, node);
            for (linked_list_node *lln = node->call_expr.arguments->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        case A_PARAMETER_EXPR:
            // [x] check params for duplicates
            if (symbol_table_insert(current_scope, node->parameter.identifier->primary_expr.identifier_name, node)){
                to_error("Parameter name already declared in function definition", node);
            }
            break;
        default:
            printf("error -> scope: Unknown ast kind");
            break;
    }    
    return;
}


int scopecheck(struct AST_node *root, linked_list *ll){
    current_scope = create_symbol_table(NULL, NULL);
    current_scope->global = current_scope;
    calls = linked_list_new();
    scope_errors = ll;

    //scopecheck ast node
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_scope((struct AST_node *) lln->data);
    }

    //scope check calls for out of order definitions
    for (linked_list_node *lln = calls->head; lln != NULL; lln = lln->next) {
        struct AST_node *c = ((struct AST_node *) lln->data);
        
        struct AST_node *symbol = symbol_table_get(current_scope->global, c->call_expr.identifier->primary_expr.identifier_name);
        if (!symbol) {
            to_error("call to undefined function", c);
            continue;
        }
        if (symbol->kind != A_FUNC_DEF) {
            to_error("tried to call a variable", c);
            continue;
        }
        if (c->call_expr.arguments->size != symbol->func_def.parameters->size){
            to_error("Number of arguments does not match function definition", c);
            continue;
        }
    }
    destroy_symbol_table(current_scope);

    return scope_errors->size;
}
