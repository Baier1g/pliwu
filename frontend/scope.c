#include "scope.h"
#include "symbol_table.h"
#include "oc_errors.h"

typedef struct call_info call_info;

#define SHADOW_MAX_WIDTH 5

linked_list *scope_errors;
symbol_table *current_scope; // string name -> var_info
short is_in_function = 0;
short nesting_depth = 0;
short func_nesting_depth = 0;
int main_def = 0;

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
    linked_list_append(scope_errors, pack_error(n, error_msg));
}

char *scope_generate_label(char* string, int i) {
    char *tmp = calloc(strlen(string) + SHADOW_MAX_WIDTH, sizeof(char));
    sprintf(tmp, "%d%s", i, string);
    return tmp;
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
    char *name, *label;
    var_info *v;
    short l,r;
    l = r = 0;
    //printf("scope: %s\n", kind_enum_to_string(node->kind));
    switch(node->kind) {
        case A_PROGRAM:
            printf("scope.c::recurse_scope: entered with kind A_PROGRAM\n");
            break;
        case A_MODULE:
            nesting_depth++;
            func_nesting_depth++;
            outer_table = current_scope;
            node->table = create_symbol_table(current_scope, current_scope->global);
            current_scope = node->table;
            //find funcs
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                if (((AST_node *) lln->data)->kind == A_FUNC_DEF) {
                    AST_node* func_node = ((AST_node*) lln->data);
                    if (strcmp(func_node->func_def.identifier->primary_expr.identifier_name, "main") == 0) {
                        if (main_def == 0) {
                            main_def += 1;
                        } else {
                            to_error("Multiple main functions defined", func_node);
                        }
                    }
                    v = create_var_info(nesting_depth, func_nesting_depth);
                    v->kind = ID_FUNCTION;
                    v->num_params = func_node->func_def.parameters->size;
                    v->ast_node = func_node;
                    name = func_node->func_def.identifier->primary_expr.identifier_name;
                    if (symbol_table_contains(current_scope, name)){
                        to_error("function name already used", func_node);
                    } else {
                        symbol_table_insert(current_scope, name, v);
                    }
                }
            }
            //recurse block
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_scope((AST_node *) lln->data);
            }
            current_scope = outer_table;
            func_nesting_depth--;
            nesting_depth--;
            break;
        case A_FUNC_DEF:
            //check existance
            name = node->func_def.identifier->primary_expr.identifier_name;
            if (!symbol_table_contains(current_scope, name)) {
                to_error("function defined in nested scope, not nested function scope", node);
            }

            //enter func
            outer_table = current_scope;
            node->table = create_symbol_table(current_scope, current_scope->global);
            current_scope = node->table;
            func_nesting_depth++;
            nesting_depth++;
            short was_in = is_in_function;
            is_in_function = 1;
            
            //find funcs
            for (linked_list_node *lln = node->func_def.function_block->block.stmt_list->head; lln != NULL; lln = lln->next) {
                if (((AST_node *) lln->data)->kind == A_FUNC_DEF) {
                    AST_node * func_node = (AST_node *) lln->data;
                    v = create_var_info(nesting_depth, func_nesting_depth);
                    v->kind = ID_FUNCTION;
                    v->num_params = func_node->func_def.parameters->size;
                    v->ast_node = func_node;
                    name = func_node->func_def.identifier->primary_expr.identifier_name;
                    if (symbol_table_contains(current_scope, name)){
                        to_error("function name already used", func_node);
                    } else {
                        symbol_table_insert(current_scope, name, v);
                    }
                }
            }

            //add params
            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            nesting_depth--; //this fakes the nesting depth as parameters are in a separate table; recursing block increments depth. ensures static link traversal works.

            //recurse block
            r = recurse_scope(node->func_def.function_block);

            if (node->func_def.return_type == TYPE_VOID && !r) {
                //Ensure epilogue
                linked_list_append(node->func_def.function_block->block.stmt_list, create_unary_node(0, 0, A_RETURN_STMT, NULL));
            } else {
                if (!r) {
                    to_error("Function missing return statement", node);
                }
            }

            //exit func
            current_scope = outer_table;
            func_nesting_depth--;
            is_in_function = was_in;
            break;
            //----

            // [x] check name
            // [x] name -> symboltable
            v = create_var_info(nesting_depth, func_nesting_depth);
            v->kind = ID_FUNCTION;
            v->num_params = node->func_def.parameters->size;
            v->ast_node = node;
            if (symbol_table_contains(current_scope, node->func_def.identifier->primary_expr.identifier_name)){
                to_error("function name already used", node);
                return 0;
            } else {
                symbol_table_insert(current_scope, node->func_def.identifier->primary_expr.identifier_name, v);
            }
            if (strcmp(node->func_def.identifier->primary_expr.identifier_name, "main") == 0) {
                if (nesting_depth != 1) {
                    to_error("Main definition in nested scope", node);
                    return 0;
                }
                main_def++;
            }
            
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            node->table = current_scope;
            was_in = is_in_function;
            is_in_function = 1;
            func_nesting_depth++;
            nesting_depth++;
            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            nesting_depth--; //this fakes the nesting depth as parameters are in a separate table. ensures static link traversal works.

            r = recurse_scope(node->func_def.function_block);

            if (node->func_def.return_type == TYPE_VOID && !l) {
                //Ensure epilogue
                linked_list_append(node->func_def.function_block->block.stmt_list, create_unary_node(0, 0, A_RETURN_STMT, NULL));
            } else {
                if (!r) {
                    to_error("Function missing return statement", node);
                }
            }
            
            func_nesting_depth--;
            current_scope = outer_table;
            is_in_function = was_in;
            break;
        case A_VAR_DECL:
            if (node->var_decl.expr_stmt) {
                recurse_scope(node->var_decl.expr_stmt);
            }
            name = node->var_decl.identifier->primary_expr.identifier_name;
            //printf(" == vardclname: %s\n", name);
            v = (var_info *) symbol_table_get(current_scope, name);
            if (v) {
                if (v->kind == ID_FUNC_PARAM || v->kind == ID_FUNCTION && v->func_nesting_depth == func_nesting_depth) {
                    to_error("Tried to redefine function or parameter", node);
                    break;
                } else if (v->nesting_depth == nesting_depth) {
                    to_error("Variable name already in use in this scope", node);
                    break;
                }
            }
            label = scope_generate_label(name, nesting_depth);
            v = (var_info *) symbol_table_get(current_scope, label);
            v = create_var_info(nesting_depth, func_nesting_depth);
            v->kind = ID_VARIABLE;
            symbol_table_insert(current_scope, label, v);
            free(name);
            node->var_decl.identifier->primary_expr.identifier_name = label;
            break;
        case A_ARRAY_DECL:
            if (node->array_decl.values){
                recurse_scope_array(node->array_decl.values, node->array_decl.sizes->size, 1);
            }
            //check identifier, if free, insert
            name = node->array_decl.identifier->primary_expr.identifier_name;
            label = scope_generate_label(name, nesting_depth);
            v = (var_info *) symbol_table_get(current_scope, label);
            if (v) {
                if (v->kind == ID_FUNC_PARAM || v->kind == ID_FUNCTION) {
                    to_error("Tried to to redefine function or parameter", node);
                }
                if (v->nesting_depth == nesting_depth) {
                    to_error("Array name already in use in this scope", node);
                }
            }
            v = create_var_info(nesting_depth, func_nesting_depth);
            v->kind = ID_ARRAY;
            v->ast_node = node;
            symbol_table_insert(current_scope, label, v);

            free(name);
            node->array_decl.identifier->primary_expr.identifier_name = label;
            //check bracket expressions
            for (linked_list_node *lln = node->array_decl.sizes->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        case A_BLOCK_STMT:
            outer_table = current_scope;
            current_scope = create_symbol_table(current_scope, current_scope->global);
            node->table = current_scope;
            nesting_depth++;
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                l = recurse_scope(lln->data);
                if(l){r=1;}
            }
            nesting_depth--;
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
            node->table = current_scope;
            recurse_scope(node->print_stmt.expression);
            break;
        case A_EXPR_STMT:
            printf("scope.c::recurse_scope: Entered with kind A_EXPR_STMT\n");
            break;
        case A_RETURN_STMT:
            if (!is_in_function) {
                to_error("returned outside of function", node);
            }
            if (node->return_stmt.expression) {
                recurse_scope(node->return_stmt.expression);
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
                name = node->primary_expr.identifier_name;
                if (symbol_table_contains(current_scope, name)){
                    //is a func param; check escaping
                    var_info *v = (var_info *) symbol_table_get(current_scope, name);
                    if (v->func_nesting_depth != func_nesting_depth) {
                        v->escaping = 1;
                    }
                    break;
                }

                int exists = 0;
                for (int depth = nesting_depth; !exists && depth >= 0; depth--) {
                    label = scope_generate_label(name,depth);
                    if (symbol_table_contains(current_scope, label)){
                        exists = 1;
                        break;
                    }
                    free(label);
                }
                if (!exists) {
                    to_error("Undefined identifier", node);
                    break;
                }

                var_info *v = (var_info *) symbol_table_get(current_scope, label);
                if (v->kind == ID_FUNCTION) {
                    to_error("Tried to access a function when variable was expected.", node);
                    break;
                } else {
                    free(name);
                    node->primary_expr.identifier_name = label;
                }
                if (v->func_nesting_depth < func_nesting_depth) {
                    v->escaping = 1;
                }
            }
            
            break;
        case A_CALL_EXPR:
            var_info *v = (var_info*) symbol_table_get(current_scope, node->call_expr.identifier->primary_expr.identifier_name);
            if (!v) {
                to_error("call to undefined function", node);
            } else if (v->kind != ID_FUNCTION) {
                to_error("tried to call a non-function", node);
            }
            if (node->call_expr.arguments->size != v->num_params) {
                to_error("Number of arguments does not match function definition", node);
            }

            for (linked_list_node *lln = node->call_expr.arguments->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            break;
        case A_PARAMETER_EXPR:
            // [x] check params for duplicates
            name = node->parameter.identifier->primary_expr.identifier_name;

            v = create_var_info(nesting_depth, func_nesting_depth);
            v->kind = ID_FUNC_PARAM;
            if (symbol_table_insert(current_scope, name, v)) {
                to_error("Parameter name already declared in function definition", node);
            }
            break;
        case A_INDEX_EXPR:
            name = node->indexing.identifier->primary_expr.identifier_name;
            //check arrayname in scope
            recurse_scope(node->indexing.identifier);
            
            //check if indexing is in scope
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
    scope_errors = ll;
    root->table = current_scope;

    //scopecheck ast node
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_scope((AST_node *) lln->data);
    }

    if (main_def == 0) {
        to_error("Missing main function definition", NULL);
    }

    return ll->size;
}
