#include "codegen.h"

linked_list *generated_code;
symbol_table *stack_offset;
int label_counter = 0;
int offset_counter = 1;


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
    stack_offset = create_symbol_table(NULL, NULL);
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
            char* name = node->var_decl.identifier->primary_expr.identifier_name;
            symbol_table_insert(stack_offset, name, offset_counter * 8);
            offset_counter++;
            printf("offset_counter is at: %d = %d\n", offset_counter, offset_counter * 8);
            if (node->var_decl.expr_stmt) {
                generate_code_helper(node->var_decl.expr_stmt);
                linked_list_append(generated_code, "\tpush rax\n");
            }
            break;
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
            char *operations = calloc(64, sizeof(char));
            switch (node->assign_expr.expression->kind) {
                case A_ARITHMETIC_EXPR:
                    int offset = (int) symbol_table_get(stack_offset, node->assign_expr.identifier->primary_expr.identifier_name);
                    if (offset) {
                        generate_code_helper(node->assign_expr.expression);
                        sprintf(operations, "\tmov QWORD PTR [rbp-%d], rax\n", offset);
                        linked_list_append(generated_code, operations);
                    } else {
                        generate_code_helper(node->assign_expr.expression);
                        char *name = node->var_decl.identifier->primary_expr.identifier_name;
                        symbol_table_insert(stack_offset, name, offset_counter);
                        offset_counter++;
                        printf("offset_counter is at: %d = %d\n", offset_counter, offset_counter * 8);
                        printf("offset_counter is at: %d\n", offset_counter);
                        linked_list_append(generated_code, "\tpush rax");
                    }
                    break;
            }
            free(operations);
            break;
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
            switch (node->primary_expr.type) {
                case TYPE_INT:
                    printf("sprintf result (num_bytes_printed): %d\n", sprintf(operation, "\tmov rax, %d\n", node->primary_expr.integer_value));
                    linked_list_append(generated_code, operation);
                    break;
                case TYPE_CHAR:
                    printf("sprintf result (num_bytes_printed): %d\n", sprintf(operation, "\tmov rax, %c\n", node->primary_expr.char_value));
                    linked_list_append(generated_code, operation);
                    break;
                case TYPE_IDENTIFIER:
                    int offset = (int) symbol_table_get(stack_offset, node->primary_expr.identifier_name);
                    printf("offset is at: %d = %d\n", offset, offset * 8);
                    printf("identifer yo num bytes printed: %d\n", sprintf(operation, "\tmov rax, QWORD PTR [rbp-%d]\n", offset));
                    linked_list_append(generated_code, operation);
                    break;
            }       
            break;
        case A_CALL_EXPR:
            return;
        case A_PARAMETER_EXPR:
            return;
        default:
            return;
    }
}