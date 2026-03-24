#include "ir.h"

segment *current_segment;
frame *current_frame;
symbol_table *current_frame_table, *current_AST_table;
int temp_counter = 1;

char *IR_op_code_to_string(enum IR_op_code op) {
    switch (op) {
        case IR_ASSIGN:
            return "assign";
        case IR_PARAM:
            return "param";
        case IR_ADD:
            return "add";
        case IR_SUB:
            return "sub";
        case IR_MUL:
            return "mul";
        case IR_DIV:
            return "div";
        case IR_EQUALS:
            return "equals";
        case IR_NEQUALS:
            return "not_equals";
        case IR_LESS:
            return "less";
        case IR_GREATER:
            return "greater";
        case IR_LESS_EQ:
            return "less_equals";
        case IR_GREATER_EQ:
            return "greater_equals";
        case IR_AND:
            return "and";
        case IR_OR:
            return "or";
        case IR_PRINT:
            return "print";
        case IR_CALL:
            return "call";
        case IR_RET:
            return "return";
        case IR_GOTO:
            return "goto";
        default:
            return "Unkown op_code in op_code_to_string";
    }
}

enum IR_op_code AST_op_to_IR_op(binary_op op) {
    switch(op) {
        case A_LESS:
            return IR_LESS;
        case A_GREATER:
            return IR_GREATER;
        case A_LESS_EQ:
            return IR_LESS_EQ;
        case A_GREATER_EQ:
            return IR_GREATER_EQ;
        case A_EQUALS:
            return IR_EQUALS;
        case A_NEQUALS:
            return IR_NEQUALS;
        case A_ADD:
            return IR_ADD;
        case A_SUB:
            return IR_SUB;
        case A_MULT:
            return IR_MUL;
        case A_DIV:
            return IR_DIV;
        case A_AND:
            return IR_AND;
        case A_OR:
            return IR_OR;
    }
}

IR_operation *create_op(enum IR_op_code op, IR_operand *arg1, IR_operand *arg2, IR_operand *arg3) {
    IR_operation *oper = malloc(sizeof(IR_operation));
    oper->op = op;
    oper->arg1 = arg1;
    oper->arg2 = arg2;
    oper->arg3 = arg3;
    return oper;
}

frame *create_frame() {
    frame *tmp = (frame *) malloc(sizeof(frame));
    tmp->name = NULL;
    tmp->nested_frames = linked_list_new();
    return tmp;
}

frame *create_named_frame(char *name) {
    frame *tmp = (frame *) malloc(sizeof(frame));
    tmp->name = calloc(strlen(name), sizeof(char));
    strcpy(tmp->name, name);
    tmp->nested_frames = linked_list_new();
    return tmp;
}

segment *create_segment(symbol_table *table) {
    segment *tmp = malloc(sizeof(segment));
    tmp->left = tmp->right = NULL;
    tmp->table = table;
    tmp->operations = linked_list_new();
    return tmp;
}

IR_operand *create_operand(operand_type type, void *content) {
    IR_operand *op = malloc(sizeof(IR_operand));
    op->type = type;
    switch(type) {
        case P_VARIABLE:
            op->variable_name = calloc(strlen((char *) content), sizeof(char));
            strcpy(op->variable_name, content);
        case P_TEMP:
        case P_CONSTANT:
            op->constant = (int) content;
            break;
        case P_LABEL:
            op->dest = (segment *) content;
        case P_FUNC_CALL:
            op->call = (frame *) content;
    }
}

int recurse_IR_tree(AST_node *node) {
    if (!node) {
        return 0;
    }

    //printf("Current node kind is %s\n", kind_enum_to_string(node->kind));

    symbol_table *sym, *old_table;
    frame *frm, *old_frame;
    segment *seg, *old_segment;
    IR_operation *op;
    IR_operand *expr, *id;
    switch (node->kind) {
        case A_MODULE:
            frm = create_frame();
            frm->segment = create_segment(node->table);
            linked_list_append(current_frame->nested_frames, frm);
            sym = create_symbol_table(current_frame_table, current_frame_table->global);
            old_table = current_frame_table;
            current_frame_table = sym;
            old_frame = current_frame;
            old_segment = current_segment;
            current_frame = frm;
            current_segment = frm->segment;
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                AST_node *n;
                //printf("Made it to %s in module\n", kind_enum_to_string(((AST_node *)lln->data)->kind));
                if (((AST_node *) lln->data)->kind == A_FUNC_DEF) {
                    n = (AST_node *) lln->data;
                    char *name = n->func_def.identifier->primary_expr.identifier_name;
                    //printf("got name\n");
                    frame *fr = create_named_frame(name);
                    //printf("Created frame\n");
                    linked_list_append(current_frame->nested_frames, fr);
                    //printf("appended frame\n");
                    symbol_table_insert(current_frame_table, name, fr);
                }
            }
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
            current_frame_table = old_table;
            current_frame = old_frame;
            current_segment = old_segment;
            break;
        case A_FUNC_DEF:
            frm = symbol_table_get(current_frame_table, node->func_def.identifier->primary_expr.identifier_name);
            frm->segment = create_segment(node->table);
            sym = create_symbol_table(current_frame_table, current_frame_table->global);
            old_table = current_frame_table;
            current_frame_table = sym;
            old_frame = current_frame;
            old_segment = current_segment;
            current_frame = frm;
            current_segment = frm->segment;
            for (linked_list_node *lln = node->func_def.function_block->block.stmt_list->head; lln != NULL; lln = lln->next) {
                AST_node *n;
                if ((n = ((AST_node *) lln->data))->kind == A_FUNC_DEF) {
                    char *name = n->func_def.identifier->primary_expr.identifier_name;
                    frame *fr = create_named_frame(name);
                    linked_list_append(current_frame->nested_frames, fr);
                    symbol_table_insert(current_frame_table, name, fr);
                }
            }
            
            recurse_IR_tree(node->func_def.function_block);
            current_frame_table = old_table;
            current_frame = old_frame;
            current_segment = old_segment;
            break;
        case A_VAR_DECL:
            id = create_operand(P_VARIABLE, node->var_decl.identifier->primary_expr.identifier_name);
            expr = NULL;
            if (node->var_decl.expr_stmt) {
                expr = create_operand(P_TEMP, recurse_IR_tree(node->var_decl.expr_stmt));
            }
            op = create_op(IR_VAR_DECL, id, expr, NULL);
            //print_operation(op);
            linked_list_append(current_segment->operations, op);
            break;
        case A_BLOCK_STMT:
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
            print_IR(current_segment);
            break;
        case A_IF_STMT:
            recurse_IR_tree(node->if_stmt.condition);
            current_segment->left = create_segment(node->if_stmt.if_branch->table);
            current_segment->right = create_segment(node->if_stmt.else_branch->table);
            seg = create_segment(node->table);
            old_segment = current_segment;

            current_segment = current_segment->left;
            recurse_IR_tree(node->if_stmt.if_branch);
            
            current_segment->left = seg;

            current_segment = old_segment->right;
            recurse_IR_tree(node->if_stmt.else_branch);
            current_segment->left = seg;
            current_segment = seg;
            break;
        case A_PRINT_STMT:
            expr = NULL;
            if (node->print_stmt.expression) {
                expr = create_operand(P_TEMP, recurse_IR_tree(node->print_stmt.expression));
            }
            op = create_op(IR_PRINT, expr, NULL, NULL);
            linked_list_append(current_segment->operations, op);
            break;
        case A_EXPR_STMT:
            recurse_IR_tree(node->expr_stmt.expression);
            break;
        case A_RETURN_STMT:
            expr = NULL;
            if (node->return_stmt.expression) {
                expr = create_operand(P_TEMP, recurse_IR_tree(node->return_stmt.expression));
            }
            op = create_op(IR_RET, expr, NULL, NULL);
            linked_list_append(current_segment->operations, op);
            break;
        case A_ASSIGN_EXPR:
            expr = create_operand(P_TEMP, recurse_IR_tree(node->assign_expr.expression));
            id = create_operand(P_VARIABLE, node->assign_expr.identifier->primary_expr.identifier_name);
            op = create_op(IR_ASSIGN, id, expr, NULL);
            linked_list_append(current_segment->operations, op);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            IR_operand *left = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.left));
            IR_operand *right = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.right));
            enum IR_op_code op_code = AST_op_to_IR_op(node->binary_expr.op);

            op = create_op(op_code, create_operand(P_TEMP, temp_counter), left, right);
            linked_list_append(current_segment->operations, op);
            return temp_counter++;
        case A_UNARY_EXPR:
            // TODO: VERY UNFINISHED
            recurse_IR_tree(node->unary_expr.expression);
            break;
        case A_PRIMARY_EXPR:
            int val = 0;
            switch (node->primary_expr.type) {
                case TYPE_CHAR:
                    val = (int) node->primary_expr.char_value;
                    break;
                case TYPE_INT:
                    val = node->primary_expr.integer_value;
                    break;
                case TYPE_BOOL:
                    val = node->primary_expr.bool_value;
                    break;
            }

            IR_operand *tmp = create_operand(P_TEMP, temp_counter);
            if (node->primary_expr.type == TYPE_IDENTIFIER) {
                op = create_op(IR_ASSIGN, tmp,
                    create_operand(P_VARIABLE, node->primary_expr.identifier_name), NULL);
            } else {
                op = create_op(IR_ASSIGN, tmp, create_operand(P_CONSTANT, val), NULL);
            }
            linked_list_append(current_segment->operations, op);
            return temp_counter++;
            break;
        case A_CALL_EXPR:
            linked_list *ll = node->call_expr.arguments;
            for (linked_list_node *lln = ll->head; lln != NULL; lln = lln->next) {
                IR_operand *arg = create_operand(P_TEMP, recurse_IR_tree((AST_node *) lln->data));
                op = create_op(IR_PARAM, arg, NULL, NULL);
                linked_list_append(current_segment->operations, op);
            }
            frame *called_func = (frame *) symbol_table_get(current_frame_table, node->call_expr.identifier->primary_expr.identifier_name);
            op = create_op(IR_CALL, create_operand(P_FUNC_CALL, called_func), NULL, NULL);
            linked_list_append(current_segment->operations, op);
            break;
        case A_PARAMETER_EXPR:
            
            break;
        default:
            printf("ir.c::recurse_IR_tree: unknown AST_node kind");
            break;
    }
    return 0;
}

void print_operand(IR_operand *op) {
    char *name = IR_op_code_to_string(op->type);
    switch (op->type) {
        case P_VARIABLE:
            printf("%s", op->variable_name);
            break;
        case P_TEMP:
            printf("T");
        case P_CONSTANT:
            printf("%d", op->constant);
            break;
        case P_FUNC_CALL:
            printf("%s", op->call->name);
            break;
        case P_LABEL:
            printf("wuh?");
    }
}

void print_operation(IR_operation *op) {
    char *name = IR_op_code_to_string(op->op);
    switch (op->op) {
        case IR_ADD:
        case IR_SUB:
        case IR_MUL:
        case IR_DIV:
        case IR_EQUALS:
        case IR_NEQUALS:
        case IR_LESS:
        case IR_GREATER:
        case IR_LESS_EQ:
        case IR_GREATER_EQ:
        case IR_AND:
        case IR_OR:
            print_operand(op->arg1);
            printf(" <- %s ", name);
            print_operand(op->arg2);
            printf(", ");
            print_operand(op->arg3);
            break;
        case IR_VAR_DECL:
            print_operand(op->arg1);
            if (op->arg2) {
                printf(" <- ");
                print_operand(op->arg2);
            }
            break;
        case IR_ASSIGN:
            print_operand(op->arg1);
            printf(" <- ");
            print_operand(op->arg2);
            break;
        case IR_CALL:
            printf("%s ", name);
            print_operand(op->arg1);
            break;
        case IR_PRINT:
        case IR_RET:
            printf("%s ", name);
            if (op->arg1) {
                print_operand(op->arg1);
            }
            break;
        case IR_PARAM:
            printf("%s ", name);
            print_operand(op->arg1);
            break;
        default:
            

    }
    printf("\n");
}

void print_IR(segment *segment) {
    if (!segment) {
        return;
    }
    IR_operation *op;
    for (linked_list_node *lln = segment->operations->head; lln != NULL; lln = lln->next) {
        op = (IR_operation *) lln->data;
        print_operation(op);
    }
    print_IR(segment->left);
    print_IR(segment->right);
}

void print_IR_tree(frame *root) {
    if (root->name) {
        printf("%s:\n", root->name);
    }
    print_IR(root->segment);
    for (linked_list_node *lln = root->nested_frames->head; lln != NULL; lln = lln->next) {
        print_IR_tree((frame *) lln->data);
    }
}


frame *create_IR_tree(AST_node *root) {
    current_frame_table = create_symbol_table(NULL, NULL);
    frame *global_frame = create_named_frame("/PROGRAM");
    global_frame->segment = create_segment(root->table);
    current_frame = global_frame;
    current_segment = global_frame->segment;
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_IR_tree((AST_node *) lln->data);
    }
    if (strcmp("/PROGRAM", current_frame->name) != 0) {
        printf("You serve A LOT of purpose, you should love yourself NOW!\n");
        exit(2);
    }
    printf("Finished IR creation\n");
    return global_frame;
}