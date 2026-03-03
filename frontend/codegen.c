#include "codegen.h"

linked_list *generated_code, *functions;
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

char *decide_branching(struct AST_node *node) {
    // JUMPS for if statements, returns the opposite instruction since we jump to the else label
    switch(node->binary_expr.op) {
        case A_LESS:
            return "\tjge ";
        case A_GREATER:
            return "\tjle ";
        case A_EQUALS:
            return "\tjne ";
        case A_NEQUALS:
            return "\tje ";
        case A_GREATER_EQ:
            return "\tjl ";
        case A_LESS_EQ:
            return "\tjg ";
        default:
            return NULL;
    }
}

char *generate_label(char* string, int i) {
    char *tmp = calloc(strlen(string) + 5, sizeof(char));
    sprintf(tmp, "%s%d", string, i);
    return tmp;
}

void create_print_macro(void) {
    linked_list_append(generated_code, \
    "%macro sys_write 3\n\tmov rdi, %1\n\tmov rsi, %2\n\tmov rdx, %3\n\tmov rax,1\n\tsyscall\n%endmacro\n\n");
}

void create_print_int(void) {
    // Print prelude
    linked_list_append(generated_code, \
    "print_int:\n\tpush rbp\n\tmov rbp, rsp\n\tpush rdi\n\tmov rdi, output\n\tlea r10, [rsp-1]\n\txor rcx, rcx\n");
    // prelude p2: newline and setup
    linked_list_append(generated_code, \
    "\tmov byte[r10], 0xa\n\tdec r10\n\tinc rcx\n\tmov rax, qword[rbp-8]\n\tmov r8, 10\n");
    // Processing the integer
    linked_list_append(generated_code, \
    "L1:\n\txor rdx, rdx\n\tdiv r8\n\tmov r9b, [table+rdx]\n\tmov [r10], r9b\n\tdec r10\n\tinc rcx\n\ttest rax, rax\n\tjnz L1\n");
    // Printing the integer
    linked_list_append(generated_code, \
    "\tlea rsi, [r10+1]\n\tcld\n_L1:\n\tmovsb\n\tcmp rsi, rsp\n\tjne _L1\n\tpop rdi\n\tsys_write 1, output, rcx\n");
    // Epilogue
    linked_list_append(generated_code, \
    "\tmov rsp, rbp\n\tpop rbp\n\tret\n\n");
}

void generate_code(linked_list *ll, struct AST_node *node) {
    generated_code = ll;
    functions = linked_list_new();
    create_print_macro();
    stack_offset = create_symbol_table(NULL, NULL);
    generate_code_helper(node);
    // finish shit ig
    linked_list_append(generated_code, \
    "\tmov rsp, rbp\n\tpop rbp\n\tmov rax, 1\n\tint 0x80\n\n");
    for (linked_list_node *lln = functions->head; lln != NULL; lln = lln->next) {
        linked_list_append(generated_code, (char*) lln->data);
        printf("%s", (char *) lln->data);
    }
}

void generate_code_helper(struct AST_node *node) {
    char *operations = NULL;
    char *op = NULL;
    int i;
    switch(node->kind) {
        case A_PROGRAM:
            linked_list_append(generated_code, \
                "section .bss\n\toutput resb 256\n");
            linked_list_append(generated_code, \
                "section .data\n\ttable db '0123456789'\n\tnewline db 0xa\nsection .text\n\n");
            create_print_int();
            linked_list_append(generated_code, \
                "global _start\n_start:\n\tpush rbp\n\tmov rbp, rsp\n");
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
            printf("entering function\n");
            int old_offset = offset_counter;
            offset_counter = -(node->func_def.parameters->size);
            symbol_table *old = stack_offset;
            stack_offset = create_symbol_table(stack_offset, NULL);
            linked_list *old_ll = generated_code;
            generated_code = functions;
            //prologue
            i = 0;
            char* function_name = generate_label(node->func_def.identifier->primary_expr.identifier_name, 0);
            linked_list_append(generated_code, function_name);
            linked_list_append(generated_code, ":\n\tpush rbp\n\tmov rbp, rsp\n");

            /* DOGSHIT
            for (linked_list_node lln = node->func_def.parameters.head; i < 6; lln = lln->next) {
                symbol_table_insert(arg_name, offset_counter * 8);
                offset_counter++;
                ll.append("push %s", arg_registers)
                i++;
            }
            int rbp_offset = 1;
            while (i < node->func_def.parameters->size) {
                symbol_table_insert(arg_name, offset_counter * 8);
                offset_counter++;
                ll.append("mov rax, qword[rbp+rbp_offset*i++]\n\tpush rax")
            } DOGSHIT END*/

            for (linked_list_node *lln = node->func_def.parameters->tail; lln != NULL; lln = lln->prev) {
                symbol_table_insert(stack_offset, (char *) lln->data, offset_counter * 8);
                offset_counter++;
            }
            offset_counter++;

            generate_code_helper(node->func_def.function_block);

            linked_list_append(generated_code, "\tmov rsp, rbp\n\tpop rbp\n\tret\n\n");
            destroy_symbol_table(stack_offset);
            stack_offset = old;
            offset_counter = old_offset;
            generated_code = old_ll;
            printf("exiting function\n");
            break;
        case A_VAR_DECL:
            char* name = node->var_decl.identifier->primary_expr.identifier_name;
            printf("offset_counter is at: %d = %d\n", offset_counter, offset_counter * 8);
            if (node->var_decl.expr_stmt) {
                symbol_table_insert(stack_offset, name, offset_counter * 8);
                offset_counter++;
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
            i = label_counter++;
            char *else_label = generate_label("else", i);
            char *end_if_label = generate_label("end_if", i);
            printf("entering if branch\n");
            generate_code_helper(node->if_stmt.condition);
            printf("exiting if branch\n");
            linked_list_append(generated_code, decide_branching(node->if_stmt.condition));
            if (node->if_stmt.else_branch) {
                linked_list_append(generated_code, else_label);
                linked_list_append(generated_code, "\n");
            } else {
                linked_list_append(generated_code, end_if_label);
                linked_list_append(generated_code, "\n");
            }
            generate_code_helper(node->if_stmt.if_branch);
            linked_list_append(generated_code, "\tjmp ");
            linked_list_append(generated_code, end_if_label);
            linked_list_append(generated_code, "\n");
            if (node->if_stmt.else_branch) {
                linked_list_append(generated_code, else_label);
                linked_list_append(generated_code, ":\n");
                printf("entering else branch\n");
                generate_code_helper(node->if_stmt.else_branch);
                printf("exiting else branch\n");
            }
            linked_list_append(generated_code, generate_label("end_if", i));
            linked_list_append(generated_code, ":\n");
            label_counter++;
            break;
        case A_PRINT_STMT:
            generate_code_helper(node->print_stmt.expression);
            linked_list_append(generated_code, \
            "\tmov rdi, rax\n\tpush rax\n\tcall print_int\n\tpop rax\n");
            break;
        case A_EXPR_STMT:
            return;
        case A_RETURN_STMT:
            printf("entering return\n");
            if (node->return_stmt.expression) {
                generate_code_helper(node->return_stmt.expression);
            }
            printf("exiting return\n");
            break;
        case A_ASSIGN_EXPR:
            printf("entering assign\n");
            operations = calloc(64, sizeof(char));
            if (!operations) {
                exit(-2);
            }
            int offset = symbol_table_get(stack_offset, node->assign_expr.identifier->primary_expr.identifier_name);
            if (offset) {
                printf("got offset, it is: %d", offset);
                generate_code_helper(node->assign_expr.expression);
                sprintf(operations, "\tmov qword[rbp-%d], rax\n", offset);
                linked_list_append(generated_code, operations);
            } else {
                generate_code_helper(node->assign_expr.expression);
                char *name = node->var_decl.identifier->primary_expr.identifier_name;
                symbol_table_insert(stack_offset, name, offset_counter * 8);
                printf("offset_counter is at: %d = %d\n", offset_counter, offset_counter * 8);
                //printf("offset_counter is at: %d\n", offset_counter);
                linked_list_append(generated_code, "\tpush rax");
                offset_counter++;
            }
            break;
        case A_LOGICAL_EXPR:
            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.left);
            switch (node->binary_expr.op) {
                case A_AND:
                    linked_list_append(generated_code,\
                        "\tpop rbx\n\tcmp rax, rbx\n");
            }
            linked_list_append(generated_code,\
                "\tpop rbx\n\tcmp rax, rbx\n");
            break;
        case A_RELATIONAL_EXPR:
            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.left);
            linked_list_append(generated_code,\
                "\tpop rbx\n\tcmp rax, rbx\n");
            break;
        case A_ARITHMETIC_EXPR:
            generate_code_helper(node->binary_expr.left);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.right);
            switch (node->binary_expr.op) {
                case A_MULT:
                    linked_list_append(generated_code,\
                        "\tpop rbx\n\timul rax, rbx\n");
                    break;
                case A_ADD:
                    linked_list_append(generated_code,\
                        "\tpop rbx\n\tadd rax, rbx\n");
                    break;
                case A_SUB:
                    linked_list_append(generated_code,\
                        "\tpop rbx\n\tsub rax, rbx\n");
                    break;
            }
            break;
        case A_UNARY_EXPR:
            return;
        case A_PRIMARY_EXPR:
            op = calloc(30, sizeof(char));
            switch (node->primary_expr.type) {
                case TYPE_INT:
                    //printf("sprintf result (num_bytes_printed): %d\n", sprintf(op, "\tmov rax, %d\n", node->primary_expr.integer_value));
                    sprintf(op, "\tmov rax, %d\n", node->primary_expr.integer_value);
                    linked_list_append(generated_code, op);
                    break;
                case TYPE_CHAR:
                    //printf("sprintf result (num_bytes_printed): %d\n", sprintf(op, "\tmov rax, %c\n", node->primary_expr.char_value));
                    sprintf(op, "\tmov rax, %d\n", (int) node->primary_expr.char_value);
                    linked_list_append(generated_code, op);
                    break;
                case TYPE_IDENTIFIER:
                    int offset = (int) symbol_table_get(stack_offset, node->primary_expr.identifier_name);
                    //printf("offset is at: %d\n", offset);
                    //printf("identifer yo num bytes printed: %d\n", sprintf(op, "\tmov rax, qword[rbp-%d]\n", offset));
                    sprintf(op, "\tmov rax, qword[rbp-%d]\n", offset);
                    linked_list_append(generated_code, op);
                    break;
                case TYPE_BOOL:
                    sprintf(op, "\tmov rax, %d\n", node->primary_expr.bool_value);
                    linked_list_append(generated_code, op);
                    break;
            }       
            break;
        case A_CALL_EXPR:
            printf("entering call\n");
            i = 0;
            /* DOGSHIT
            for each (parameters) {
                eval(parameter);
                if (i<6) {
                    ll.append("push %s", arg_registers)
                    ll.append("mov %s, rax", arg_registers[i++]);
                } else {
                    break;
                }
            }
            ll.append("call %s", node->call.identifier->primary_expr.identifier_name);
            while (i >= 6) {
                ll.append("pop")
                i--;
            }
            while (i <= 0) {
                ll.append("pop %s", arg_registers[i--]);
            }
             DOGSHIT END*/
            linked_list *params = node->call_expr.arguments;
            for (linked_list_node *lln = params->tail; lln != NULL; lln = lln->prev) {
                generate_code_helper((struct AST_node *) lln->data);
                linked_list_append(generated_code, "\tpush rax\n");
                i++;
            }
            linked_list_append(generated_code, "\tcall ");
            linked_list_append(generated_code, node->call_expr.identifier->primary_expr.identifier_name);
            linked_list_append(generated_code, "0\n");
            linked_list_append(generated_code, "\tadd rsp, ");
            char *number = calloc(100, sizeof(char));
            int j = 0;
            i = params->size * 8;
            int k = i;
            while (k != 0) {
                k = k /10;
                j++;
            }
            j--;
            printf("Integer is %d\n", i);
            while (i != 0) {
                number[j--] = (char) ((i % 10) + '0');
                printf("%c", (char)((i % 10) + '0'));
                i = i / 10;
            }
            printf("string is %s\n", number);
            linked_list_append(generated_code, number);
            linked_list_append(generated_code, "\n");
            printf("exiting call\n");
            break;
        case A_PARAMETER_EXPR:
            printf("entering parameter\n");
            generate_code_helper(node->parameter.identifier);
            printf("exiting parameter\n");
            break;
        default:
            return;
    }
}