#include "type_checking.h"

symbol_table *current_scope;
linked_list *errors;
data_type current_return_type = TYPE_VOID;


void to_error(char *error_msg, struct AST_node *node) {
    linked_list_append(errors, error_msg);
}

data_type recurse_type(struct AST_node *node) {
    if (!node) {
        return TYPE_VOID;
    }
    char* name;
    data_type d_type, d_type_right;
    kind type = node->kind;
    symbol_table* outer_table;
    switch (type) {
        case A_MODULE:
            outer_table = current_scope;
            current_scope = node->table;
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_type((struct AST_node *) lln->data);
            }
            current_scope = outer_table;
            break;
        case A_FUNC_DEF:
            // set returntype / funcname-> returntype
            // set return type
            // inc scopes
            // set parameter types
            // recurse

            name = node->func_def.identifier->primary_expr.identifier_name;
            d_type = node->func_def.return_type;

            ((var_info*) symbol_table_get(current_scope, name))->type = d_type;


            data_type old_return_type = current_return_type;
            current_return_type = d_type;
            outer_table = current_scope;
            current_scope = node->table;

            //set param types
            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            
            recurse_type(node->func_def.function_block);
    
            //
            current_scope = outer_table;
            current_return_type = old_return_type;
            break;
        case A_VAR_DECL:
            name = node->var_decl.identifier->primary_expr.identifier_name;
            if (node->var_decl.expr_stmt) {
                data_type expr_type = recurse_type(node->var_decl.expr_stmt);
                if (expr_type != node->var_decl.type) {
                    to_error("Trying to declare a variable of a different type.", node);
                }
            }
            ((var_info *) symbol_table_get(current_scope, name))->type = node->var_decl.type;
            return node->var_decl.type;
        case A_BLOCK_STMT:
            outer_table = current_scope;
            current_scope = node->table;
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                recurse_scope(lln->data);
            }
            current_scope = outer_table;
            break;
        case A_IF_STMT:
            d_type = recurse_type(node->if_stmt.condition);
            if (!(d_type == TYPE_BOOL || d_type == TYPE_INT)){
                to_error("IF-condition of incorret type", node);
            }
            recurse_type(node->if_stmt.if_branch);
            recurse_type(node->if_stmt.else_branch);
            break;
        case A_PRINT_STMT:
            recurse_type(node->print_stmt.expression);
            break;
        case A_EXPR_STMT:
            recurse_type(node->expr_stmt.expression);
            break;
        case A_RETURN_STMT:
            d_type = recurse_type(node->return_stmt.expression);
            if (d_type != current_return_type){
                to_error("Incorrect return type", node);
            }
            break;
        case A_ASSIGN_EXPR:
            name = node->assign_expr.identifier->primary_expr.identifier_name;
            d_type = ((var_info *) symbol_table_get(current_scope, name))->type;
            if (d_type != recurse_type(node->assign_expr.expression)) {
                to_error("Trying to assign to a variable of a different type", node);
            }
            return type;
        case A_LOGICAL_EXPR:
            d_type = recurse_type(node->binary_expr.left);
            d_type_right = recurse_type(node->binary_expr.right);

            if (!(d_type == TYPE_INT || d_type == TYPE_BOOL)){
                to_error("Logical operand of incorret type", node);
            } else if (!(d_type_right == TYPE_INT || d_type_right == TYPE_BOOL)){
                to_error("Logical operand of incorret type", node);
            }
            return TYPE_BOOL;
        case A_RELATIONAL_EXPR:
            d_type = recurse_type(node->binary_expr.left);
            d_type_right = recurse_type(node->binary_expr.right);

            if (d_type != d_type_right) {
                to_error("Relational operands of different types", node);
                return TYPE_BOOL;
            }

            switch (node->binary_expr.op){
                case A_EQUALS:
                case A_NEQUALS:
                    if (!(d_type == TYPE_BOOL || d_type == TYPE_INT)){
                        to_error("Illegal type for relational expression", node);
                    }
                    break;
                default:
                    if (d_type != TYPE_INT){
                        to_error("Illegal type for relational expression", node);
                    }
                    break;
            }
            return TYPE_BOOL;
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
                return ((var_info *) (symbol_table_get(current_scope, node->primary_expr.identifier_name)))->type;
            } else {
                return node->primary_expr.type;
            }
            break;
        case A_CALL_EXPR:
            //check args
            //return func type

            
            break;
        case A_PARAMETER_EXPR:
            name = node->parameter.identifier->primary_expr.identifier_name;
            d_type = node->parameter.type;
            ((var_info *) symbol_table_get(current_scope, name))->type = d_type;
            return d_type;
        default:
            printf("This should not happen :thinking:");
            break;
    }
    return TYPE_VOID;
}

int typecheck(struct AST_node *root, linked_list *ll) {
    errors = ll;
    current_scope = root->table;
    for (linked_list_node *lln = ll->head; lln != NULL; lln = lln->next) {
        recurse_type((struct AST_node *) lln->data);
    }
    return ll->size;
}