#include "codegen.h"

linked_list *generated_code;
int label_counter = 0;

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
        case MUL:
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

void generate_code(linked_list *ll, struct AST_node *node) {
    generated_code = ll;
    generate_code_helper(node);
}

void generate_code_helper(struct AST_node *node) {
    switch(node->kind) {
        case A_PROGRAM:
            linked_list_append(generated_code, \
                "section .data\nsection .text\nglobal _start\n_start:\n");
            for (linked_list_node *n = node->module.module_declarations->head; n != NULL; n = n->next) {
                generate_code_helper(n->data);
            }
            break;
        case A_MODULE:
            for (linked_list_node *n = node->module.module_declarations->head; n != NULL; n = n->next) {
                generate_code_helper(n->data);
            }
            break;
        case A_FUNC_DEF:
            return;
        case A_VAR_DECL:
            return;
        case A_BLOCK_STMT:
            for (linked_list_node *n = node->block.stmt_list->head; n != NULL; n = n->next) {
                generate_code_helper(n->data);
            }
            break;
        case A_IF_STMT:
            int i = label_counter++;
            generate_code_helper(node->if_stmt.condition);
            linked_list_append(generated_code, "jge else\n");
            generate_code_helper(node->if_stmt.if_branch);
            linked_list_append(generated_code, "jmp end_if\n");
            if (node->if_stmt.else_branch) {
                linked_list_append(generated_code, "else:\n");
                generate_code_helper(node->if_stmt.else_branch);
            }
            linked_list_append(generated_code, \
            "end_if:\n");
            break;
        case A_PRINT_STMT:
            return;
        case A_EXPR_STMT:
            return;
        case A_RETURN_STMT:
            return;
        case A_ASSIGN_EXPR:
            return;
        case A_LOGICAL_EXPR:
            return;
        case A_RELATIONAL_EXPR:
            generate_code_helper(node->binary_expr.left);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, \
                "\tpop rbx\n\tcmp rax, rbx\n");
            break;
        case A_ARITHMETIC_EXPR:
            generate_code_helper(node->binary_expr.left);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, \
                "\tpop rbx\n\tadd rax, rbx\n");
            break;
        case A_UNARY_EXPR:
            return;
        case A_PRIMARY_EXPR:
            char *operation = calloc(30, sizeof(char));
            printf("Did we fail? %d\n", sprintf(operation, "\tmov rax, %d\n", node->primary_expr.integer_value));
            printf("Buffer is now %s\n", operation);
            linked_list_append(generated_code, operation);
            break;
        case A_CALL_EXPR:
            return;
        case A_PARAMETER_EXPR:
            return;
        default:
            return;
    }
}