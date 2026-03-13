#include "ir.h"

operation *current_op;

char *op_code_to_string(op_code op) {
    switch (op) {
        case MOV:
            return "mov";
        case PUSH:
            return "push";
        case POP:
            return "pop";
        case ADD:
            return "add";
        case ADC:
            return "adc";
        case SUB:
            return "sub";
        case IMUL:
            return "imul";
        case DIV:
            return "div";
        case CMP:
            return "cmp";
        case XOR:
            return "XOR";
        case JMP:
            return "jmp";
        case JNE:
            return "jne";
        case JE:
            return "je";
        case JG:
            return "jg";
        case JL:
            return "jl";
    }
}

operation *create_op(op_code op, operand *arg1, operand *arg2, char *comment) {
    operation *oper = malloc(sizeof(operation));
    oper->op = op;
    oper->arg1 = arg1;
    oper->arg2 = arg2;
    oper->comment = comment;
    return oper;
}

frame *create_frame() {
    return (frame *) malloc(sizeof(frame));
}

void link_operations(operation *op1, operation *op2) {
    op1->next = op2;
    op2->prev = op1;
}

operand *create_operand(operand_type type, void *content) {
    operand *op = malloc(sizeof(operand));
    op->type = type;
    switch(type) {
        case TEMP:
        case CONSTANT:
            op->constant = (int) content;
            break;
        case LABEL:
            op->dest = (operation *) content;
    }
}

void recurse_IR_tree(AST_node *node) {
    if (!node) {
        return;
    }

    switch (node->kind) {
        case A_MODULE:
            for (linked_list_node *lln = node->module.module_declarations->head; lln != NULL; lln = lln->next) {
                recurse_IR_tree((AST_node *) lln->data);
            }
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
            // MAKE IF
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
            // MAKE ASSIGN
            break;
        case A_LOGICAL_EXPR:
        case A_RELATIONAL_EXPR:
        case A_ARITHMETIC_EXPR:
            // MAKE BINARIES
            break;
        case A_UNARY_EXPR:
            // MAKE UNARY
            break;
        case A_PRIMARY_EXPR:
            // MAKE PRIMARIES
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
    return;
}

operation *create_IR_tree(AST_node *root) {
    operation *op = create_op(NULL, NULL, NULL, NULL);
    current_op = op;
    for (linked_list_node *lln = root->program.modules->head; lln != NULL; lln = lln->next) {
        recurse_IR_tree((AST_node *) lln->data);
    }
    return op;
}