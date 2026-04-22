#include "optimiser.h"

AST_node *CF_fold_children(AST_node *node) {
    int i, j;
    i = j = 0;

    AST_node *tmp = AST_optimiser_constant_folding(node->binary_expr.left);
    if (tmp && tmp->kind == A_PRIMARY_EXPR) {
        node->binary_expr.left = tmp;
        i = 1;
    }
    tmp = AST_optimiser_constant_folding(node->binary_expr.right);
    if (tmp && tmp->kind == A_PRIMARY_EXPR) {
        node->binary_expr.right = tmp;
        j = 1;
    }

    if (i && j) {
        return AST_optimiser_constant_folding(node);
    } else {
        return NULL;
    }
}

AST_node *AST_optimiser_constant_folding(AST_node *node) {
    if (!node) {
        return NULL;
    }
    kind type = node->kind;
    AST_node *tmp;
    switch (type) {
        case A_PROGRAM:
            for (linked_list_node *lln = node->program.modules->head; lln != NULL; lln = lln->next) {
                AST_optimiser_constant_folding((AST_node *) lln->data);
            }
            break;
        case A_MODULE:
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                AST_optimiser_constant_folding((AST_node *) lln->data);
            }
            break;
        case A_FUNC_DEF:
            AST_optimiser_constant_folding(node->func_def.function_block);
            break;
        case A_VAR_DECL:
            tmp = AST_optimiser_constant_folding(node->var_decl.expr_stmt);
            if (tmp) {
                node->var_decl.expr_stmt = tmp;
            }
            break;
        case A_BLOCK_STMT:
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                AST_optimiser_constant_folding((AST_node *) lln->data);
            }
            break;
        case A_IF_STMT:
            tmp = AST_optimiser_constant_folding(node->if_stmt.condition);
            if (tmp) {
                node->if_stmt.condition = tmp;
            }
            AST_optimiser_constant_folding(node->if_stmt.if_branch);
            AST_optimiser_constant_folding(node->if_stmt.else_branch);
            break;
        case A_WHILE_LOOP:
            tmp = AST_optimiser_constant_folding(node->while_loop.condition);
            if (tmp) {
                node->if_stmt.condition = tmp;
            }
            AST_optimiser_constant_folding(node->while_loop.block);
            break;
        case A_PRINT_STMT:
            tmp = AST_optimiser_constant_folding(node->print_stmt.expression);
            if (tmp) {
                node->print_stmt.expression = tmp;
            }
            break;
        case A_EXPR_STMT:
            tmp = AST_optimiser_constant_folding(node->expr_stmt.expression);
            if (tmp) {
                node->expr_stmt.expression = tmp;
            }
            break;
        case A_RETURN_STMT:
            tmp = AST_optimiser_constant_folding(node->return_stmt.expression);
            if (tmp) {
                node->return_stmt.expression = tmp;
            }
            break; 
        case A_ASSIGN_EXPR:
            tmp = AST_optimiser_constant_folding(node->assign_expr.expression);
            if (tmp) {
                node->assign_expr.expression = tmp;
            }
            break;
        case A_LOGICAL_EXPR:
            if (node->binary_expr.left->kind == A_PRIMARY_EXPR && node->binary_expr.right->kind == A_PRIMARY_EXPR) {
                int temp;

                AST_node *n1 = AST_optimiser_constant_folding(node->binary_expr.left);
                AST_node *n2 = AST_optimiser_constant_folding(node->binary_expr.right);
                if (!n1 || !n2) {
                    return NULL;
                }

                int arg1 = n1->primary_expr.bool_value;
                int arg2 = n2->primary_expr.bool_value;
                switch (node->binary_expr.op) {
                    case A_AND:
                        temp = arg1 && arg2;
                        break;
                    case A_OR:
                        temp = arg1 || arg2;
                        break;
                    default:
                        printf("optimiser.c::AST_optimiser_constant_folding: Wrong binary_op\n");
                        exit(1);
                }
                tmp = create_binary_node(node->pos.startchar, node->pos.line, A_PRIMARY_EXPR, TYPE_BOOL, temp);
                kill_tree(node);
                return tmp;
            } else {
                return CF_fold_children(node);
            }
        case A_RELATIONAL_EXPR:
            if (node->binary_expr.left->kind == A_PRIMARY_EXPR && node->binary_expr.right->kind == A_PRIMARY_EXPR) {
                int temp;
                AST_node *n1 = AST_optimiser_constant_folding(node->binary_expr.left);
                AST_node *n2 = AST_optimiser_constant_folding(node->binary_expr.right);
                if (!n1 || !n2) {
                    return NULL;
                }
                int arg1 = node->binary_expr.left->primary_expr.integer_value;
                int arg2 = node->binary_expr.right->primary_expr.integer_value;
                switch (node->binary_expr.op) {
                    case A_LESS:
                        temp = arg1 < arg2;
                        break;
                    case A_GREATER:
                        temp = arg1 > arg2;
                        break;
                    case A_GREATER_EQ:
                        temp = arg1 >= arg2;
                        break;
                    case A_LESS_EQ:
                        temp = arg1 <= arg2;
                        break;
                    case A_EQUALS:
                        temp = arg1 == arg2;
                        break;
                    case A_NEQUALS:
                        temp = arg1 != arg2;
                        break;
                    default:
                        printf("optimiser.c::AST_optimiser_constant_folding: Wrong binary_op\n");
                        exit(1);
                }
                tmp = create_binary_node(node->pos.startchar, node->pos.line, A_PRIMARY_EXPR, TYPE_BOOL, temp);
                kill_tree(node);
                return tmp;
            } else {
                return CF_fold_children(node);
            }
        case A_ARITHMETIC_EXPR:
            if (node->binary_expr.left->kind == A_PRIMARY_EXPR && node->binary_expr.right->kind == A_PRIMARY_EXPR) {
                int temp;

                AST_node *n1 = AST_optimiser_constant_folding(node->binary_expr.left);
                AST_node *n2 = AST_optimiser_constant_folding(node->binary_expr.right);
                if (!n1 || !n2) {
                    return NULL;
                }

                int arg1 = node->binary_expr.left->primary_expr.integer_value;
                int arg2 = node->binary_expr.right->primary_expr.integer_value;
                switch (node->binary_expr.op) {
                    case A_ADD:
                        temp = arg1 + arg2;
                        break;
                    case A_SUB:
                        temp = arg1 - arg2;
                        break;
                    case A_MULT:
                        temp = arg1 * arg2;
                        break;
                    case A_DIV:
                        // TODO: Do error handling for division by 0 cases
                        if (arg2 == 0) {
                            printf("optimiser::constant_folding: division by zero in line %d at character %d\n", node->pos.line, node->pos.startchar);
                            arg2 = 1;
                        }
                        temp = arg1 / arg2;
                        break;
                    default:
                        printf("optimiser.c::AST_optimiser_constant_folding: Wrong binary_op\n");
                        exit(1);
                }
                tmp = create_binary_node(node->pos.startchar, node->pos.line, A_PRIMARY_EXPR, TYPE_INT, temp);
                kill_tree(node);
                return tmp;
            } else {
                return CF_fold_children(node);
            }
            return tmp;
        case A_UNARY_EXPR:
            AST_optimiser_constant_folding(node);
            break;
        case A_PRIMARY_EXPR:
            if (node->primary_expr.type != TYPE_IDENTIFIER) {
                return node;
            }
            return NULL;
            break;
        case A_CALL_EXPR:
            linked_list *ll = node->call_expr.arguments;
            for (linked_list_node *lln = ll->head; lln != NULL; lln = lln->next) {
                tmp = AST_optimiser_constant_folding((AST_node *) lln->data);
                if (tmp) {
                    lln->data = tmp;
                }
            }
            break;
        case A_PARAMETER_EXPR:
            break;
        default:
            printf("optimiser.c::AST_optimiser_constant_folding: Unknown AST kind\n");
            break;
    }
    return NULL;
}

AST_node *AST_optimiser_dead_code_elimination(AST_node *node) {
    // TO BE IMPLEMENTED
    return NULL;
}