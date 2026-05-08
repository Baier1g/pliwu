#include "scope.h"
#include "symbol_table.h"

#define MAX_ARGUMENTS 64

typedef struct call_info call_info;

linked_list *scope_errors;
symbol_table *current_scope; // string name -> ast_node
linked_list *calls;          // list->data = ast_node
short is_in_function = 0;
short nesting_depth = 0;

struct call_info {
    AST_node *node;
    symbol_table *called_scope;
};

short recurse_scope(AST_node*);


call_info *create_call_info(AST_node *node, symbol_table *table) {
    call_info *tmp = calloc(1, sizeof(call_info));
    tmp->node = node;
    tmp->called_scope = table;
    return tmp;
}

void to_error(char *error_msg, AST_node *n) {
    //find line #, find startchar with helper
    linked_list_append(scope_errors, error_msg);
}

void recurse_scope_array(linked_list *values, int depth, int current_depth) {
    if (current_depth == depth) {
        for (linked_list_node *lln = values->head; lln != NULL; lln = lln->next) {
            recurse_scope(lln->data);
        }
    } else {
        for (linked_list_node *lln = values->head; lln != NULL; lln = lln->next) {
            recurse_scope_array(lln->data, depth, current_depth+1);
        }
    }
}

short recurse_scope(AST_node *node) {
    symbol_table *outer_table;
    char *name;
    var_info *v;
    short l,r;
    l = r = 0;
    switch(node->kind) {
        case A_PROGRAM:
            printf("scope.c::recurse_scope: entered with kind A_PROGRAM\n");
            break;
        case A_MODULE:
            nesting_depth++;
            node->table = create_symbol_table(current_scope, current_scope->global);
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_scope((AST_node *) lln->data);
            }

            nesting_depth--;
            break;
        case A_FUNC_DEF:
            // [x] check if scope is global scope
            // [x] check param maxlength
            // [x] check name
            // [x] name -> symboltable
            /*if (current_scope != (current_scope->global)){
                to_error("function definition not in global scope", node);
                return;
            }*/
            if (node->func_def.parameters->size > MAX_ARGUMENTS){
                to_error("funciton definition has too many parameters", node);
                return 0;
            }

            v = create_var_info(nesting_depth);
            v->kind = ID_FUNCTION;
            v->num_params = node->func_def.parameters->size;
            v->ast_node = node;
            if (symbol_table_insert(current_scope, node->func_def.identifier->primary_expr.identifier_name, v)){
                to_error("function name already used", node);
                return 0;
            }
            
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            node->table = current_scope;
            short was_in = is_in_function;
            is_in_function = 1;
            nesting_depth++;

            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            l = recurse_scope(node->func_def.function_block);
            if (node->func_def.return_type == TYPE_VOID) {
                linked_list_append(node->func_def.function_block->block.stmt_list, create_unary_node(0, 0, A_RETURN_STMT, NULL));
            } else {
                if (!l) {
                    to_error("Function missing return statement", node);
                }
            }
            
            nesting_depth--;
            current_scope = outer_table;
            is_in_function = was_in;
            break;
        case A_VAR_DECL:
            if (node->var_decl.expr_stmt) {
                recurse_scope(node->var_decl.expr_stmt);
            }
            v = (var_info *) symbol_table_get(current_scope, node->var_decl.identifier->primary_expr.identifier_name);
            if (!v) {
                v = create_var_info(nesting_depth);
                v->kind = ID_VARIABLE;
                symbol_table_insert(current_scope, node->var_decl.identifier->primary_expr.identifier_name, v);
            } else {
                if (v->kind == ID_FUNC_PARAM || v->kind == ID_FUNCTION || v->kind == ID_ARRAY) {
                    to_error("Tried to to redefine function or parameter or array", node);
                }
                if (symbol_table_insert(current_scope, node->var_decl.identifier->primary_expr.identifier_name, v)) {
                    to_error("Variable name already in use in this scope", node);
                }
            }
            break;
        case A_ARRAY_DECL:
            if (node->array_decl.values){
                recurse_scope_array(node->array_decl.values, node->array_decl.sizes->size, 1);
            }
            //check identifier, if free, insert
            v = (var_info *) symbol_table_get(current_scope, node->array_decl.identifier->primary_expr.identifier_name);
            if (!v) {
                v = create_var_info(nesting_depth);
                v->kind = ID_ARRAY;
                v->ast_node = node;
                symbol_table_insert(current_scope, node->array_decl.identifier->primary_expr.identifier_name, v);
            } else {
                if (v->kind == ID_FUNC_PARAM || v->kind == ID_FUNCTION || v->kind == ID_VARIABLE) {
                    to_error("Tried to to redefine function or parameter or variable", node);
                }
                if (symbol_table_insert(current_scope, node->array_decl.identifier->primary_expr.identifier_name, v)) {
                    to_error("Array name already in use in this scope", node);
                }
            }
            //check bracket expressions
            for (linked_list_node *lln = node->array_decl.sizes->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        case A_BLOCK_STMT:
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            node->table = current_scope;
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                l = recurse_scope(lln->data);
                if(l){r=1;}
            }
            current_scope = outer_table;
            if (r) {return 1;}
            break;
        case A_IF_STMT:
            recurse_scope(node->if_stmt.condition);
            
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            node->table = current_scope;
            
            recurse_scope(node->if_stmt.if_branch);

            if (node->if_stmt.else_branch) {
                recurse_scope(node->if_stmt.else_branch);
            }
            
            current_scope = outer_table;
            if (l && r) {return 1;}
            break;
        case A_WHILE_LOOP:
            recurse_scope(node->while_loop.condition);
            
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            node->table = current_scope;

            recurse_scope(node->while_loop.block);
            current_scope = outer_table;
            break;
        case A_PRINT_STMT:
            recurse_scope(node->print_stmt.expression);
            break;
        case A_EXPR_STMT:
            printf("scope.c::recurse_scope: Entered with kind A_EXPR_STMT\n");
            break;
        case A_RETURN_STMT:
            if (!is_in_function) {
                to_error("returned outside of function", node);
            }
            return 1;
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
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                if (!symbol_table_contains(current_scope, node->primary_expr.identifier_name)){
                    to_error("Undefined identifier", node);
                    break;
                }
                var_info *v = (var_info *) symbol_table_get(current_scope, node->primary_expr.identifier_name);
                if (v->kind == ID_FUNCTION) {
                    to_error("Tried to access a function when variable was expected.", node);
                    break;
                }
                if (v->nesting_depth < nesting_depth) {
                    v->escaping = 1;
                }
            }
            break;
        case A_CALL_EXPR:
            linked_list_append(calls, create_call_info(node, current_scope));
            for (linked_list_node *lln = node->call_expr.arguments->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        case A_PARAMETER_EXPR:
            // [x] check params for duplicates
            name = node->parameter.identifier->primary_expr.identifier_name;
            v = create_var_info(nesting_depth);
            v->kind = ID_FUNC_PARAM;
            if (symbol_table_insert(current_scope, name, v)) {
                to_error("Parameter name already declared in function definition", node);
            }
            break;
        case A_INDEX_EXPR:
            name = node->indexing.identifier->primary_expr.identifier_name;
            recurse_scope(node->indexing.identifier);
            //check arrayname in scope
            v = (var_info *) symbol_table_get(current_scope, name);
            if(!v){
                to_error("Tried accessing undeclared array", node);
            }
            //check brackets in scope
            for (linked_list_node *lln = node->indexing.indices->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        default:
            printf("scope.c::recurse_scope: Unknown ast kind\n");
            to_error("Unknown AST_kind", node);
            break;
    }    
    return 0;
}


int scopecheck(AST_node *root, linked_list *ll){
    current_scope = create_symbol_table(NULL, NULL);
    current_scope->global = current_scope;
    calls = linked_list_new();
    scope_errors = ll;
    root->table = current_scope;

    //scopecheck ast node
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_scope((AST_node *) lln->data);
    }

    //scope check calls for out of order definitions
    for (linked_list_node *lln = calls->head; lln != NULL; lln = lln->next) {
        //printf("calls\n");
        call_info *c = ((call_info *) lln->data);
        AST_node *node = c->node;
        var_info *v = symbol_table_get(c->called_scope, node->call_expr.identifier->primary_expr.identifier_name);
        if (!v) {
            to_error("call to undefined function", c->node);
            continue;
        }
        if (v->kind != ID_FUNCTION) {
            to_error("tried to call a variable", c->node);
            continue;
        }
        if (node->call_expr.arguments->size != v->num_params){
            to_error("Number of arguments does not match function definition", c->node);
            continue;
        }
    }
    //printf("calls done\n");

    return ll->size;
}
