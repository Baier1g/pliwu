#include "ir.h"

segment *current_segment;
frame *current_frame;
symbol_table *current_frame_table, *current_AST_table;
int temp_counter = 1;

char *op_code_to_string(op_code op) {
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

op_code AST_op_to_IR_op(binary_op op) {
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
        default:
            return "Unkown operator";
    }
}

operation *create_op(op_code op, operand *arg1, operand *arg2, operand *arg3) {
    operation *oper = malloc(sizeof(operation));
    oper->op = op;
    oper->arg1 = arg1;
    oper->arg2 = arg2;
    oper->arg3 = arg3;
    return oper;
}

frame *create_frame() {
    return (frame *) malloc(sizeof(frame));
}

frame *create_named_frame(char *name) {
    frame *tmp = create_frame();
    tmp->name = calloc(strlen(name), sizeof(char));
    strcpy(tmp->name, name);
    return tmp;
}

segment *create_segment(symbol_table *table) {
    segment *tmp = malloc(sizeof(segment));
    tmp->left = tmp->right = NULL;
    tmp->table = table;
    tmp->operations = linked_list_new();
    return tmp;
}

operand *create_operand(operand_type type, void *content) {
    operand *op = malloc(sizeof(operand));
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
        return;
    }
    frame *frm, *old_frame;
    segment *seg, *old_segment;
    operation *op;
    switch (node->kind) {
        case A_MODULE:
            frm = create_frame();
            frm->segment = create_segment(node->table);
            old_frame = current_frame;
            old_segment = current_segment;
            current_frame = frm;
            current_segment = frm->segment; 
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
            current_frame = old_frame;
            current_segment = old_segment;
            break;
        case A_FUNC_DEF:
            // MAKE FUNCTIONS
            break;
        case A_VAR_DECL:
            // MAKE VAR_DECL
            break;
        case A_BLOCK_STMT:
            for (linked_list_node *lln = node->block.stmt_list->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
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
            // MAKE PRINT
            break;
        case A_EXPR_STMT:
            // MAKE EXPR_STMT
            break;
        case A_RETURN_STMT:
            // MAKE RETURN
            break;
        case A_ASSIGN_EXPR:
            operand *expr = create_operand(P_TEMP, recurse_IR_tree(node->assign_expr.expression));
            operand *id = create_operand(P_VARIABLE, node->assign_expr.identifier->primary_expr.identifier_name);
            op = create_op(IR_ASSIGN, id, expr, NULL);
            linked_list_append(current_segment->operations, op);
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            operand *left = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.left));
            operand *right = create_operand(P_TEMP, recurse_IR_tree(node->binary_expr.right));
            op_code op_code = AST_op_to_IR_op(node->binary_expr.op);

            op = create_op(op_code, create_operand(P_TEMP, temp_counter), left, right);
            linked_list_append(current_segment->operations, op);
            return temp_counter++;
        case A_UNARY_EXPR:
            // MAKE UNARY
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

            operand *tmp = create_operand(P_TEMP, temp_counter);
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
            // MAKE CALL
            break;
        case A_PARAMETER_EXPR:
            // MAKE PARAMETER
            break;
        default:
            printf("ir.c::recurse_IR_tree: unknown AST_node kind");
            break;
    }
    return 0;
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
    return global_frame;
}