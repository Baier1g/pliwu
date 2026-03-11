#include "type_checking.h"

symbol_table *current_scope;
linked_list *errors;

void to_error(char *error_msg, struct AST_node *node) {
    linked_list_append(errors, error_msg);
}

data_type recurse_type(struct AST_node *node) {
    if (!node) {
        return;
    }
    char* name;
    data_type type;
    kind type = node->kind;
    switch (type) {
        case A_MODULE:
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_type((struct AST_node *) lln->data);
            }
            break;
        case A_FUNC_DEF:
            
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
            name = node->assign_expr.identifier->primary_expr.identifier_name;
            type = ((var_info *) symbol_table_get(current_scope, name))->type;
            if (type != recurse_type(node->assign_expr.expression)) {
                to_error("Trying to assign to a variable of a different type", node);
            }
            return type;
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
                return ((var_info *) (symbol_table_get(current_scope, node->primary_expr.identifier_name)))->type;
            } else {
                return node->primary_expr.type;
            }
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

int typecheck(struct AST_node *root, linked_list *ll) {
    errors = ll;
    for (linked_list_node *lln = ll->head; lln != NULL; lln = lln->next) {
        recurse_type((struct AST_node *) lln->data);
    }
    return ll->size;
}