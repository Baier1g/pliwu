#include "codegen.h"

linked_list *generated_code, *functions;
symbol_table *stack_offset;
int label_counter = 0;
int offset_counter = 1;
int frame_depth = 0;
int epilogue_counter = 0;
int bool_counter = 0;
int in_function = 0;

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

char *decide_branching(AST_node *node) {
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
        case A_AND:
            return "\tje ";
        case A_OR:
            return "\tjne ";
        default:
            return NULL;
    }
}

char *generate_label(char* string, int i) {
    char *tmp = calloc(strlen(string) + 5, sizeof(char));
    sprintf(tmp, "%s%d", string, i);
    return tmp;
}

/* 
 * Helper function used for debugging.
 * Generates assembly for printing the contents of rax
 */
void print_rax() {
    linked_list_append(generated_code,
                       "\tmov rdi, rax\t\t\t\t; Move argument to be printed from rax to rdi\n\tpush rax\t\t\t\t\t; Save value to be printed to the stack\n\tcall print_int\t\t\t\t; Call the print function\n\tpop rax\t\t\t\t\t\t; Restore the printed value\n");
    return;
}

/*
 * Generates the assembly for the print macro used by print_int
 */
void create_print_macro(void) {
    linked_list_append(generated_code, \
    "%macro sys_write 3\n\tmov rdi, %1\n\tmov rsi, %2\n\tmov rdx, %3\n\tmov rax,1\n\tsyscall\n%endmacro\n\n");
}

/*
 * Generates the assembly for the print_int function.
 * So far, it only handles unsigned integers
 */
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

/*
 * Main entry point of code emission
 */
void generate_code(linked_list *ll, AST_node *node) {
    generated_code = ll;
    functions = linked_list_new();
    create_print_macro();
    stack_offset = node->table;
    generate_code_helper(node);
    // finish shit ig
    linked_list_append(generated_code, \
    "\tmov rsp, rbp\n\tpop rbp\n\tmov rax, 1\n\tint 0x80\n\n");
    for (linked_list_node *lln = functions->head; lln != NULL; lln = lln->next) {
        linked_list_append(generated_code, (char*) lln->data);
        //printf("%s", (char *) lln->data);
    }
}

/*
 * Function that handles code generation for all different types of AST_node
 */
void generate_code_helper(AST_node *node) {
    char *operations = NULL;
    char *op = NULL;
    char *name = NULL;
    int i;

    var_info *var, *func;
    symbol_table *old_table;

    switch(node->kind) {
        case A_PROGRAM:
            linked_list_append(generated_code, \
                "section .bss\n\toutput resb 256\n");
            linked_list_append(generated_code, \
                "section .data\n\ttable db '0123456789'\n\tnewline db 0xa\nsection .text\n\n");
            create_print_int();
            linked_list_append(generated_code, \
                "global _start\n_start:\n\tmov rbp, rsp\n");
            //print_rax();
            for (linked_list_node *n = node->module.module_declarations->head; n != NULL; n = n->next) {
                generate_code_helper(n->data);
            }
            break;
        case A_MODULE:
            frame_depth++;
            for (linked_list_node *n = node->module.module_declarations->head; n != NULL; n = n->next) {
                generate_code_helper(n->data);
            }
            frame_depth--;
            break;
        case A_FUNC_DEF:
            //printf("entering function\n");
            int old_offset = offset_counter;
            offset_counter = -(node->func_def.parameters->size + 2);
            old_table = stack_offset;
            stack_offset = node->table;
            linked_list *old_ll = generated_code;
            generated_code = functions;
            //prologue
            i = 0;
            char* function_name = generate_label(node->func_def.identifier->primary_expr.identifier_name, 0);
            if (in_function) {
                linked_list_append(generated_code, "\tjmp end_");
                linked_list_append(generated_code, function_name);
                linked_list_append(generated_code, "\n");
            }
            linked_list_append(generated_code, function_name);
            linked_list_append(generated_code, ":\n\tpush rbp\t\t\t\t\t; Save the old base pointer\n\tmov rbp, rsp\t\t\t\t; Set up base pointer for new stack frame\n");  
            //print_rax();

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
                char *name = (char *) ((AST_node *) lln->data)->parameter.identifier->primary_expr.identifier_name;
                ((var_info *) symbol_table_get(stack_offset, name))->offset = offset_counter * 8;

                //printf("offset for %s is: %d\n", name, ((var_info *) symbol_table_get(stack_offset, name))->offset);
                offset_counter++;
            }
            offset_counter += 3;

            short was_in = in_function;
            in_function = 1;
            frame_depth++;
            generate_code_helper(node->func_def.function_block);
            frame_depth--;
            in_function = was_in;

            op = generate_label("epilogue", epilogue_counter++);
            linked_list_append(generated_code, op);
            linked_list_append(generated_code, ":\n\tmov rsp, rbp\t\t\t\t; Restore the old stack pointer before exit\n\tpop rbp\t\t\t\t\t\t; Restore the base pointer of the previous stack\n\tret\n\n");
            if (in_function) {
                linked_list_append(generated_code, "end_");
                linked_list_append(generated_code, function_name);
                linked_list_append(generated_code, ":\n");
            }
            stack_offset = old_table;
            offset_counter = old_offset;
            generated_code = old_ll;
            //printf("exiting function\n");
            break;
        case A_VAR_DECL:
            name = node->var_decl.identifier->primary_expr.identifier_name;
            //printf("offset_counter is at: %d = %d\n", offset_counter, offset_counter * 8);
            if (node->var_decl.expr_stmt) {
                var = symbol_table_get(stack_offset, name);
                var->offset = offset_counter * 8;
                //printf("offset is %d for var %s in var_decl\n", offset_counter * 8, name);
                offset_counter++;
                generate_code_helper(node->var_decl.expr_stmt);
                linked_list_append(generated_code, "\tpush rax\n");
            }
            break;
        case A_BLOCK_STMT:
            int old_c;
            if (in_function) {
                old_table = stack_offset;
                stack_offset = node->table;
                old_c = offset_counter;
                //printf("offset counter is %d in block\n", offset_counter * 8);
            }
            for (linked_list_node *n = node->block.stmt_list->head; n != NULL; n = n->next) {
                generate_code_helper(n->data);
            }
            if (in_function) {
                //printf("offset counter is %d in post-block\n", offset_counter * 8);
                stack_offset = old_table;
                offset_counter = old_c;
            }
            break;
        case A_IF_STMT:
            i = label_counter++;
            char *else_label = generate_label("else", i);
            char *end_if_label = generate_label("end_if", i);
            generate_code_helper(node->if_stmt.condition);
            if (node->if_stmt.condition->kind == A_PRIMARY_EXPR) {
                linked_list_append(generated_code, "\ttest rax, rax\n\tjz ");
            } else {
                linked_list_append(generated_code, decide_branching(node->if_stmt.condition));
            }
            if (node->if_stmt.else_branch) {
                linked_list_append(generated_code, else_label);
                linked_list_append(generated_code, "\n");
            } else {
                linked_list_append(generated_code, end_if_label);
                linked_list_append(generated_code, "\n");
            }
            //printf("entering if branch\n");
            generate_code_helper(node->if_stmt.if_branch);
            //printf("exiting if branch\n");
            linked_list_append(generated_code, "\tjmp ");
            linked_list_append(generated_code, end_if_label);
            linked_list_append(generated_code, "\n");
            if (node->if_stmt.else_branch) {
                linked_list_append(generated_code, else_label);
                linked_list_append(generated_code, ":\n");
                //printf("entering else branch\n");
                generate_code_helper(node->if_stmt.else_branch);
                //printf("exiting else branch\n");
            }
            linked_list_append(generated_code, generate_label("end_if", i));
            linked_list_append(generated_code, ":\n");
            label_counter++;
            break;
        case A_WHILE_LOOP:
            i = label_counter++;
            char *while_label = generate_label("while", i);
            char *end_while_label = generate_label("end_while", i);
            linked_list_append(generated_code, while_label);
            linked_list_append(generated_code, ":\n");
            generate_code_helper(node->while_loop.condition);
            linked_list_append(generated_code, decide_branching(node->while_loop.condition));
            linked_list_append(generated_code, end_while_label);
            linked_list_append(generated_code, "\n");
            generate_code_helper(node->while_loop.block);
            linked_list_append(generated_code, "\n\tjmp ");
            linked_list_append(generated_code, while_label);
            linked_list_append(generated_code, "\n");
            linked_list_append(generated_code, end_while_label);
            linked_list_append(generated_code, ":\n");
            break;
        case A_PRINT_STMT:
            generate_code_helper(node->print_stmt.expression);
            linked_list_append(generated_code, \
            "\tmov rdi, rax\t\t\t\t; Move argument to be printed from rax to rdi\n\tpush rax\t\t\t\t\t; Save value to be printed to the stack\n\tcall print_int\t\t\t\t; Call the print function\n\tpop rax\t\t\t\t\t\t; Restore the printed value\n");
            break;
        case A_EXPR_STMT:
            return;
        case A_RETURN_STMT:
            //printf("entering return\n");
            if (node->return_stmt.expression) {
                generate_code_helper(node->return_stmt.expression);
            }
            op = generate_label("epilogue", epilogue_counter);
            linked_list_append(generated_code, "\tjmp ");
            linked_list_append(generated_code, op);
            linked_list_append(generated_code, "\n");
            //printf("exiting return\n");
            break;
        case A_ASSIGN_EXPR:
            //printf("entering assign\n");
            operations = calloc(64, sizeof(char));
            if (!operations) {
                exit(-2);
            }
            name = node->assign_expr.identifier->primary_expr.identifier_name;
            int offset = ((var_info *) symbol_table_get(stack_offset, name))->offset;
            if (offset) {
                //printf("got offset, it is: %d for var %s\n", offset, name);
                generate_code_helper(node->assign_expr.expression);
                sprintf(operations, "\tmov qword[rbp-%d], rax\n", offset);
                linked_list_append(generated_code, operations);
            } else {
                generate_code_helper(node->assign_expr.expression);
                //char *name = node->var_decl.identifier->primary_expr.identifier_name;
                ((var_info *) symbol_table_get(stack_offset, name))->offset = offset_counter * 8;
                //printf("offset_counter is at: %d for var %s in assign\n", offset_counter, name);
                //printf("offset_counter is at: %d\n", offset_counter);
                linked_list_append(generated_code, "\tpush rax\n");
                offset_counter++;
            }
            break;
        case A_LOGICAL_EXPR:
            op = generate_label("false", bool_counter);
            operations = generate_label("end_logical", bool_counter++);

            generate_code_helper(node->binary_expr.left);
            linked_list_append(generated_code, "\tcmp rax, 0\n");
            linked_list_append(generated_code, decide_branching(node));
            linked_list_append(generated_code, op);
            linked_list_append(generated_code, "\n");

            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, "\tcmp rax, 0\n");
            linked_list_append(generated_code, decide_branching(node));
            linked_list_append(generated_code, op);
            if (node->binary_expr.op == A_AND) {
                linked_list_append(generated_code, "\n\tmov rax, 1\n\tjmp ");
            } else {
                linked_list_append(generated_code, "\n\tmov rax, 0\n\tjmp ");
            }    
            linked_list_append(generated_code, operations);
            linked_list_append(generated_code, "\n");

            linked_list_append(generated_code, op);
            if (node->binary_expr.op == A_AND) {
                linked_list_append(generated_code, ":\n\tmov rax, 0");
            } else {
                linked_list_append(generated_code, ":\n\tmov rax, 1");
            }
            linked_list_append(generated_code, "\n");
            linked_list_append(generated_code, operations);
            linked_list_append(generated_code, ":\n");
            break;
        case A_RELATIONAL_EXPR:
            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.left);
            op = generate_label("false", bool_counter);
            operations = generate_label("end_rel", bool_counter++);
            linked_list_append(generated_code,\
                "\tpop rbx\n\tcmp rax, rbx\n");
            linked_list_append(generated_code, decide_branching(node));
            linked_list_append(generated_code, op);
            linked_list_append(generated_code, "\n\tmov rax, 1\n\tjmp ");
            linked_list_append(generated_code, operations);
            linked_list_append(generated_code, "\n");
            linked_list_append(generated_code, op);
            linked_list_append(generated_code, ":\n\tmov rax, 0\n");
            linked_list_append(generated_code, operations);
            linked_list_append(generated_code, ":\n");
            break;
        case A_ARITHMETIC_EXPR:
            generate_code_helper(node->binary_expr.right);
            linked_list_append(generated_code, "\tpush rax\n");
            generate_code_helper(node->binary_expr.left);
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
                case A_DIV:
                    linked_list_append(generated_code,\
                        "\txor rdx, rdx\n\tpop rbx\n\tdiv rbx\n");
                    break;
            }
            break;
        case A_UNARY_EXPR:
            return;
        case A_PRIMARY_EXPR:
            op = calloc(128, sizeof(char));
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
                    var = (var_info *) symbol_table_get(stack_offset, node->primary_expr.identifier_name);
                    int offset = var->offset;
                    int depth = var->nesting_depth;

                    //printf("offset is at: %d for var %s in primary. We are at depth %d and var depth is %d\n", offset, node->primary_expr.identifier_name, frame_depth, depth);
                    //printf("identifer yo num bytes printed: %d\n", sprintf(op, "\tmov rax, qword[rbp-%d]\n", offset));
                    if (offset > 0) {
                        if (var->nesting_depth == frame_depth) {
                            linked_list_append(generated_code, "\tlea rax, [rbp]\n");
                        } else {
                            int depth_diff = frame_depth - var->nesting_depth;
                            linked_list_append(generated_code, "\tlea rax, [rbp+16]\n\tmov rax, qword[rax]\n");
                            depth_diff--;
                            //print_rax();
                            while (depth_diff > 0) {                    //rax + 16
                                linked_list_append(generated_code, "\tlea rax, [rax+16]\n\tmov rax, qword[rax]\n");
                                //print_rax();
                                depth_diff--;
                            }
                            //linked_list_append(generated_code, "\tsub rax, 16\n");
                            //print_rax();
                        }
                        sprintf(op, "\tmov rax, qword[rax-%d]\t\t; Load the value of a variable into rax\n", offset);
                    } else {
                        offset = -(offset);
                        sprintf(op, "\tmov rax, qword[rbp+%d]\t\t; Load function argument from above base pointer\n", offset);
                    }
                    linked_list_append(generated_code, op);
                    break;
                case TYPE_BOOL:
                    sprintf(op, "\tmov rax, %d\n", node->primary_expr.bool_value);
                    linked_list_append(generated_code, op);
                    break;
            }       
            break;
        case A_CALL_EXPR:
            //printf("entering call\n");
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
                generate_code_helper((AST_node *) lln->data);
                linked_list_append(generated_code, "\tpush rax\n");
                i++;
            }
            // STATIC LINK
            var = symbol_table_get(stack_offset, node->call_expr.identifier->primary_expr.identifier_name);
            //printf("Calling %s, we are at frame depth %d and the called function has depth %d\n", node->call_expr.identifier->primary_expr.identifier_name, frame_depth, var->nesting_depth);
            if (frame_depth == var->nesting_depth) {
                linked_list_append(generated_code, "\tlea rax, [rbp]\t\t\t; Calling a nested function, static link is calling functions rbp\n");
            } else {
                linked_list_append(generated_code, "\tlea rax, [rbp+16]\t\t\t; Load the address containing the address of the static link for link traversal\n\tmov rax, qword[rax]\t\t\t; Dereference rax to get the address of the static link\n");
                //print_rax();
                int depth_diff = (frame_depth - 1) - var->nesting_depth;
                while (depth_diff > 0) {                    //rax + 16
                    linked_list_append(generated_code, "\tlea rax, [rax+16]\n\tmov rax, qword[rax]\t\t\t\t; Traversing static link\n");
                    //print_rax();
                    depth_diff--;
                }
            }
            linked_list_append(generated_code, "\tpush rax\t\t\t\t\t; Pushing static link to stack\n");

            linked_list_append(generated_code, "\tcall ");
            linked_list_append(generated_code, node->call_expr.identifier->primary_expr.identifier_name);
            linked_list_append(generated_code, "0\t\t\t\t; Calling function\n");
            linked_list_append(generated_code, "\tadd rsp, ");
            char *number = calloc(100, sizeof(char));
            int j = 0;
            i = (params->size + 1) * 8;
            int k = i;
            while (k != 0) {
                k = k /10;
                j++;
            }
            j--;
            //printf("Integer is %d\n", i);
            while (i != 0) {
                number[j--] = (char) ((i % 10) + '0');
                //printf("%c", (char)((i % 10) + '0'));
                i = i / 10;
            }
            //printf("string is %s\n", number);
            linked_list_append(generated_code, number);
            linked_list_append(generated_code, "\t\t\t\t\t; Reset stack pointer after call, getting rid of function arguments\n");
            //printf("exiting call\n");
            break;
        case A_PARAMETER_EXPR:
            //printf("entering parameter\n");
            generate_code_helper(node->parameter.identifier);
            //printf("exiting parameter\n");
            break;
        default:
            return;
    }
}
