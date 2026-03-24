#include "type_checking.h"

symbol_table *type_scope;
linked_list *type_errors;
data_type current_return_type = TYPE_VOID;


void type_to_error(char *error_msg, AST_node *node) {
    linked_list_append(type_errors, error_msg);
}

data_type recurse_type(AST_node *node) {
    if (!node) {
        return TYPE_VOID;
    }
    char* name;
    data_type d_type, d_type_right;
    kind type = node->kind;
    symbol_table* outer_table;
    
    switch (type) {
        case A_MODULE:
            outer_table = type_scope;
            type_scope = node->table;
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_type((AST_node *) lln->data);
            }
            type_scope = outer_table;
            break;
        case A_FUNC_DEF:
            // set function returntype
            // set current returntype
            // increment scopes
            // set parameter types
            // recurse block

            name = node->func_def.identifier->primary_expr.identifier_name;
            d_type = node->func_def.return_type;

            ((var_info*) symbol_table_get(type_scope, name))->type = d_type;


            data_type old_return_type = current_return_type;
            current_return_type = d_type;
            outer_table = type_scope;
            type_scope = node->table;

            //set param types
            for (linked_list_node *lln = node->func_def.parameters->head; lln != NULL; lln = lln->next) {
                recurse_type(lln->data);
            }
            
            recurse_type(node->func_def.function_block);
            type_scope = outer_table;
            current_return_type = old_return_type;
            break;
        case A_VAR_DECL:
            name = node->var_decl.identifier->primary_expr.identifier_name;
            if (node->var_decl.expr_stmt) {
                data_type expr_type = recurse_type(node->var_decl.expr_stmt);
                if (expr_type != node->var_decl.type) {
                    type_to_error("Trying to declare a variable of a different type.", node);
                }
            }
            ((var_info *) symbol_table_get(type_scope, name))->type = node->var_decl.type;
            return node->var_decl.type;
        case A_BLOCK_STMT:
            outer_table = type_scope;
            type_scope = node->table;
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                recurse_type(lln->data);
            }
            type_scope = outer_table;
            break;
        case A_IF_STMT:
            d_type = recurse_type(node->if_stmt.condition);
            if (!(d_type == TYPE_BOOL || d_type == TYPE_INT)){
                type_to_error("IF-condition of incorrect type", node);
            }
            recurse_type(node->if_stmt.if_branch);
            recurse_type(node->if_stmt.else_branch);
            break;
        case A_WHILE_LOOP:
            d_type = recurse_type(node->while_loop.condition);
            if (!(d_type == TYPE_BOOL || d_type == TYPE_INT)){
                type_to_error("while-condition of incorrect type", node);
            }
            recurse_type(node->while_loop.block);
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
                type_to_error("Incorrect return type", node);
            }
            break;
        case A_ASSIGN_EXPR:
            name = node->assign_expr.identifier->primary_expr.identifier_name;
            d_type = ((var_info *) symbol_table_get(type_scope, name))->type;
            if (d_type != recurse_type(node->assign_expr.expression)) {
                type_to_error("Trying to assign to a variable of a different type", node);
            }
            return type;
        case A_LOGICAL_EXPR:
            d_type = recurse_type(node->binary_expr.left);
            d_type_right = recurse_type(node->binary_expr.right);

            if (!(d_type == TYPE_INT || d_type == TYPE_BOOL)){
                type_to_error("Logical operand of incorrect type", node);
            } else if (!(d_type_right == TYPE_INT || d_type_right == TYPE_BOOL)){
                type_to_error("Logical operand of incorrect type", node);
            }
            return TYPE_BOOL;
        case A_RELATIONAL_EXPR:
            d_type = recurse_type(node->binary_expr.left);
            d_type_right = recurse_type(node->binary_expr.right);

            if (d_type != d_type_right) {
                type_to_error("Relational operands of different types", node);
                return TYPE_BOOL;
            }

            switch (node->binary_expr.op){
                case A_EQUALS:
                case A_NEQUALS:
                    if (!(d_type == TYPE_BOOL || d_type == TYPE_INT)){
                        type_to_error("Illegal type for relational expression", node);
                    }
                    break;
                default:
                    if (d_type != TYPE_INT){
                        type_to_error("Illegal type for relational expression", node);
                    }
                    break;
            }
            return TYPE_BOOL;
        case A_ARITHMETIC_EXPR:
            d_type = recurse_type(node->binary_expr.left);
            d_type_right = recurse_type(node->binary_expr.right);
            if (d_type != TYPE_INT || d_type_right != TYPE_INT){
                type_to_error("Arithmetic operand of incorrect type", node);
            }
            return TYPE_INT;
        case A_UNARY_EXPR:
            d_type = recurse_type(node->unary_expr.expression);
            switch (node->unary_expr.op){
                case A_NEG:
                    if(d_type != TYPE_BOOL){
                        type_to_error("Unary negation used on non-boolean", node);
                    }
                case A_MINUS:
                    if(d_type != TYPE_INT){
                        type_to_error("Unary minus used on non-integer", node);
                    }
                break;
            default:
                type_to_error("Typecheck: Unhandled operator case in unary", node);
                break;
            }
            break;
        case A_PRIMARY_EXPR:
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                return ((var_info *) (symbol_table_get(type_scope, node->primary_expr.identifier_name)))->type;
            } else {
                return node->primary_expr.type;
            }
            break;
        case A_CALL_EXPR:
            //check args
            //return func type

            name = node->call_expr.identifier->primary_expr.identifier_name;
            AST_node * funcnode = ((var_info*) symbol_table_get(type_scope, name))->ast_node;
            linked_list_node *param_node = funcnode->func_def.parameters->head;
            
            for (linked_list_node *lln = node->call_expr.arguments->head; lln != NULL; lln = lln->next) {
                d_type = recurse_type(lln->data);
                d_type_right = ((AST_node *) param_node->data)->parameter.type;

                if (d_type != d_type_right) {
                    type_to_error("Argument type does not match parameter type", node);
                    break;//to break or not to break
                }
                param_node = param_node->next;
            }
            return funcnode->func_def.return_type;
        case A_PARAMETER_EXPR:
            name = node->parameter.identifier->primary_expr.identifier_name;
            d_type = node->parameter.type;
            ((var_info *) symbol_table_get(type_scope, name))->type = d_type;
            return d_type;
        default:
            printf("type_checking.c::recurse_type: Unrecognized ast node");
            break;
    }
    return TYPE_VOID;
}

int typecheck(AST_node *root, linked_list *ll) {
    type_errors = ll;
    type_scope = root->table;
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_type((AST_node *) lln->data);
    }
    return ll->size;
}