#include "ir_codegen.h"

linked_list *CG_generated_code, *module_level, *data_section;
segment *CG_current_segment;
frame *CG_current_frame;
int CG_offset = 1;
int CG_reg_max = 0;
int CG_frame_depth = 0;
int relational_counter = 0;
int logical_counter = 0;
int epilogue_count = 0;
int CG_string_counter = 0;
int CG_array_counter = 0;

//int offset_counter = 0;

char *CG_reg_color_to_string(reg_color reg) {
    switch (reg) {
        case R15:
            return "r15";
        case R14:
            return "r14";
        case R13:
            return "r13";
        case R12:
            return "r12";
        case R11:
            return "r11";
        case R10:
            return "r10";
        case R9:
            return "r9";
        case R8:
            return "r8";
        case RAX:
            return "rax";
        case RBX:
            return "rbx";
        case RDI:
            return "rdi";
        case RSI:
            return "rsi";
        case RCX:
            return "rcx";
        case RDX:
            return "rdx";
        default:
            printf("ir_codegen.c::CG_reg_color_to_string: Undefined register\n");
            return NULL;
    }
}

void IR_create_print_macro(void) {
    linked_list_append(data_section, \
    "%macro sys_write 3\n\tmov rdi, %1\n\tmov rsi, %2\n\tmov rdx, %3\n\tmov rax,1\n\tsyscall\n%endmacro\n\n");
}

void IR_create_print_int(void) {
    // Print prelude
    linked_list_append(CG_generated_code, \
    "print_int:\n\tpush rbp\n\tmov rbp, rsp\n\tpush r8\n\tpush r9\n\tpush r10\n\tpush r11\n\tpush rdi\n\tmov rdi, output\n\tlea r10, [rsp-1]\n\txor rcx, rcx\n");
    // prelude p2: newline and setup
    linked_list_append(CG_generated_code, \
    "\tmov byte[r10], 0xa\n\tdec r10\n\tinc rcx\n\tmov rax, qword[rbp-40]\t\t\t\t; Access provided argument on the stack\n\tmov r8, 10\n");
    // Processing the integer
    linked_list_append(CG_generated_code, \
    "L1:\n\txor rdx, rdx\n\tdiv r8\n\tmov r9b, [table+rdx]\n\tmov [r10], r9b\n\tdec r10\n\tinc rcx\n\ttest rax, rax\n\tjnz L1\n");
    // Printing the integer
    linked_list_append(CG_generated_code, \
    "\tlea rsi, [r10+1]\n\tcld\n_L1:\n\tmovsb\n\tcmp rsi, rsp\n\tjne _L1\n\tpop rdi\n\tsys_write 1, output, rcx\n");
    // Epilogue
    linked_list_append(CG_generated_code, \
    "\tpop r11\n\tpop r10\n\tpop r9\n\tpop r8\n\tmov rsp, rbp\n\tpop rbp\n\tret\n\n");
}

void IR_create_alloc(void) {
    linked_list_append(CG_generated_code,\
    "_alloc:\n\tpush rbp\n\tmov rbp, rsp\n\tpush r8\n\tmov rax, qword[heap_pointer]\t\t\t; Move start of allocated segment into rax\n");
    linked_list_append(CG_generated_code,\
    "\tmov qword[rax], rdi\t\t\t\t; Move element size into base address\n\tmov qword[rax + 8], rsi\t\t\t; put length of allocated memory segment at the heap pointer\n\tadd qword[heap_pointer], 16\t\t\t; Increment heap pointer to next available slot\n");
    linked_list_append(CG_generated_code,\
    "\tmov r8, rsi\t\t\t\t; Move number of elements into r8\n\timul r8, 8\t\t\t\t; Multiply r8 by 8 to get total number of bytes needed\n");
    linked_list_append(CG_generated_code,\
    "\tadd qword[heap_pointer], r8\t\t\t; Add r8 to the heap_pointer to get first free space past segment\n\tpop r8\n\tmov rsp, rbp\n\tpop rbp\n\tret\n\n");
}

char *IR_decide_branching(IR_operation *operation) {
    // JUMPS for if statements, returns the opposite instruction since we jump to the else label
    switch(operation->op) {
        case IR_LESS:
            return "\tjge ";
        case IR_GREATER:
            return "\tjle ";
        case IR_EQUALS:
            return "\tjne ";
        case IR_NEQUALS:
            return "\tje ";
        case IR_GREATER_EQ:
            return "\tjl ";
        case IR_LESS_EQ:
            return "\tjg ";
        case IR_AND:
            return "\tje ";
        case IR_OR:
            return "\tjne ";
        default:
            printf("ir_codegen.c::IR_decide_branching: Unknown op code\n");
            return NULL;
    }
}

char *CG_IR_op_code_to_string(IR_op_code code) {
    switch (code) {
        case IR_ADD:
            return "\tadd";
        case IR_SUB:
            return "\tsub";
        case IR_MUL:
            return "\timul";
        case IR_DIV:
            return "\tidiv";
        case IR_GOTO:
            return "\tjmp";
        default:
            printf("ir_codegen.c::IR_op_code_to_CG: Unknown IR_op_code\n");
            return NULL;
    }
}

/*
 * Performs static link traversal and leaves the address of the variable in question in rax
 */
void CG_var_address(var_info *var) {
    //int depth = var->nesting_depth;
    int var_offset = var->offset;
    int in_frame = 0;
    
    // intermediate, non-user variables have a nesting depth of -1 by default.
    // Since they can only be accessed from the frame they were created in, their real nesting depth is CG_frame_depth
    if (var->nesting_depth == -1) {
        var->nesting_depth = CG_frame_depth;
    }
    //printf("var: %d, frame_depth: %d, offset: %d\n", var->nesting_depth, CG_frame_depth, var->offset);
    if (var->nesting_depth == CG_frame_depth) {
        in_frame = 1;
    } else {
        int depth_diff = CG_frame_depth - var->nesting_depth;
        linked_list_append(CG_generated_code, "\tlea rax, [rbp+16]\n\tmov rax, qword[rax]\n");
        depth_diff--;
        while (depth_diff > 0) {                    //rax + 16
            linked_list_append(CG_generated_code, "\tlea rax, [rax+16]\n\tmov rax, qword[rax]\n");
            depth_diff--;
        }
    }

    char *buffer = (char *) calloc(128, sizeof(char));
    if (var->kind == ID_VARIABLE) {
        // Local variable
        if (in_frame) {
            sprintf(buffer, "\tlea rax, qword[rbp-%d]\t\t; Load the value of a variable into rax\n", var_offset);
        } else {
            sprintf(buffer, "\tlea rax, qword[rax-%d]\t\t; Load the value of a variable into rax\n", var_offset);
        }
    } else {
        // Function parameter
        var_offset = -(var_offset);
        sprintf(buffer, "\tlea rax, qword[rbp+%d]\t\t; Load function argument from above base pointer\n", var_offset);
    }
    linked_list_append(CG_generated_code, buffer);
}

CG_operand *CG_create_operand(CG_operand_type type, CG_operand_mode mode, void *a, void *b) {
    CG_operand *tmp = (CG_operand *) malloc(sizeof(CG_operand));
    tmp->type = type;
    tmp->mode = mode;
    switch (type) {
        case O_REGISTER:
            tmp->reg = (reg_color) a;
            break;
        case O_CONSTANT:
            tmp->constant = (int) a;
            break;
        case O_LABEL:
            int len = strlen((char *) a);
            tmp->label = (char *) calloc(len + 3, sizeof(char));
            strncpy(tmp->label, (char *) a, len + 3);
            break;
        case O_DISPLACEMENT:
            tmp->displacement.reg = (reg_color) a;
            tmp->displacement.offset = (int) b;
            break;
        default:
            printf("ir_codegen.c::CG_create_operand: Unknown operand type\n");
            return NULL;
    }
    return tmp;
}

void CG_create_static_link(IR_operation *operation) {
    var_info *var = (var_info *) symbol_table_get(operation->in_seg->table, operation->arg2->call->name);
    //printf("Calling %s, we are at frame depth %d and the called function has depth %d\n", node->call_expr.identifier->primary_expr.identifier_name, frame_depth, var->nesting_depth);
    if (CG_frame_depth == var->nesting_depth) {
        linked_list_append(CG_generated_code, "\tlea rax, [rbp]\t\t\t; Calling a nested function, static link is calling functions rbp\n");
    } else {
        linked_list_append(CG_generated_code, "\tlea rax, [rbp+16]\t\t\t; Load the address containing the address of the static link for link traversal\n\tmov rax, qword[rax]\t\t\t; Dereference rax to get the address of the static link\n");
        //print_rax();
        int depth_diff = (CG_frame_depth - 1) - var->nesting_depth;
        while (depth_diff > 0) {                    //rax + 16
            linked_list_append(CG_generated_code, "\tlea rax, [rax+16]\n\tmov rax, qword[rax]\t\t\t\t; Traversing static link\n");
            //print_rax();
            depth_diff--;
        }
    }
    linked_list_append(CG_generated_code, "\tpush rax\t\t\t\t; Pushing static link to stack\n");
}

void recurse_segment(segment *seg, RA_graph *graph) {
    if (!seg) {
        return;
    }
    //printf("recursing segment\n");
    CG_current_segment = seg;
    if (seg->name) {
        linked_list_append(CG_generated_code, seg->name);
        linked_list_append(CG_generated_code, ":\n");
    }
    var_info *info;
    int param_count = 0;

    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        IR_operation *operation = (IR_operation *) lln->data;
        //print_operation(operation);
        IR_operation *prev;
        IR_op_code code = operation->op;
        //printf("op_code: %s\n", IR_op_code_to_string(code));
        char *name, *label, *label2;
        switch (code) {
            case IR_VAR_DECL:
                if (operation->arg1->type == P_TEMP) {
                    continue;
                }
                name = operation->arg1->variable_name;
                label = (char *) calloc(256, sizeof(char));
                info = (var_info *) symbol_table_get(seg->table, name);
                sprintf(label, "\tmov qword[rbp-%d], %s\t\t\t\t; Assign value to variable %s\n", info->offset, CG_reg_color_to_string(graph->nodes[operation->arg2->constant]->color), name);
                linked_list_append(CG_generated_code, label);
                break;
            case IR_ASSIGN:
                name = (char *) calloc(128, sizeof(char));
                //printf("henlo\n");
                if (operation->arg1->type == P_TEMP) {
                    reg_color reg = (reg_color) graph->nodes[operation->arg1->constant]->color;
                    if (operation->arg2->type == P_CONSTANT) {
                        sprintf(name, "\tmov %s, %d\t\t\t\t; Put constant value into register", CG_reg_color_to_string(reg), operation->arg2->constant);
                    } else if (operation->arg2->type == P_TEMP) {
                        sprintf(name, "\tmov %s, %s\t\t\t\t; Assign from register to register", CG_reg_color_to_string(reg), CG_reg_color_to_string(graph->nodes[operation->arg2->constant]->color));
                    } else if (operation->arg2->type == P_DEREFERENCE) {
                        sprintf(name, "\tmov %s, qword[%s]\t\t\t\t; Assign from memory to register", CG_reg_color_to_string(reg), CG_reg_color_to_string(graph->nodes[operation->arg2->constant]->color));
                    } else {
                        var_info *var = (var_info *) symbol_table_get(seg->table, operation->arg2->variable_name);
                        if (operation->arg2->variable_name[0] == '_') {
                            printf("Got here\n");
                            label = operation->arg2->variable_name;
                            sprintf(name, "\tlea %s, [%s]\t\t\t; Load starting address of string literal %s into %s\n", CG_reg_color_to_string(reg), label, label, CG_reg_color_to_string(reg));
                        } else {
                            CG_var_address(var);
                            sprintf(name, "\tmov %s, qword[rax]\t\t\t; Load value of variable into register", CG_reg_color_to_string(reg));
                        }
                    }
                } else if (operation->arg1->type == P_DEREFERENCE) {
                    reg_color reg = (reg_color) graph->nodes[operation->arg1->constant]->color;
                    reg_color reg2 = (reg_color) graph->nodes[operation->arg2->constant]->color;
                    if (operation->arg2->type == P_DEREFERENCE) {
                        label = (char *) calloc(128, sizeof(char));
                        sprintf(label, "\tmov rax, qword[%s]\t\t\t\t; Move the value of the address in %s into rax\n", CG_reg_color_to_string(reg), CG_reg_color_to_string(reg));
                        linked_list_append(CG_generated_code, label);
                        sprintf(name, "\tmov qword[%s], rax\t\t\t\t; Move value of rax into memory address pointed to by %s\n", CG_reg_color_to_string(reg), CG_reg_color_to_string(reg));
                    } else {
                        sprintf(name, "\tmov qword[%s], %s\t\t\t\t; Move value of %s into memory address pointed to by %s\n", CG_reg_color_to_string(reg), CG_reg_color_to_string(reg2), CG_reg_color_to_string(reg2), CG_reg_color_to_string(reg));
                    }
                } else {
                    char *tmp = operation->arg1->variable_name;
                    var_info *var = (var_info *) symbol_table_get(seg->table, tmp);
                    CG_var_address(var);
                    if (operation->arg2->type == P_CONSTANT) {
                        sprintf(name, "\tmov qword[rax], %d\t\t\t; Put constant value into variable", operation->arg2->constant);
                    } else {
                        reg_color reg = (reg_color) graph->nodes[operation->arg2->constant]->color;
                        sprintf(name, "\tmov qword[rax], %s\t\t\t; Load value of variable into register", CG_reg_color_to_string(reg));
                    }
                }
                linked_list_append(CG_generated_code, name);
                break;
            case IR_PARAM:
                name = (char *) calloc(128, sizeof(char));
                if (param_count < 4) {
                    if (param_count <= CG_current_frame->func_params) {
                        sprintf(name, "\tpush %s\t\t\t\t; Push current function parameter %s to the stack\n", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(param_count + 11));
                        linked_list_append(CG_generated_code, name);
                        name = (char *) calloc(128, sizeof(char));
                    }
                    if (operation->arg1->type == P_TEMP) {
                        sprintf(name, "\tmov %s, %s\t\t\t\t; Move function argument into %s", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color), CG_reg_color_to_string(param_count + 11));
                    } else {
                        var_info *info = symbol_table_get(seg->table, operation->arg1->variable_name);
                        CG_var_address(info);
                        sprintf(name, "\tmov %s, qword[rax]\t\t\t\t; Move function argument into %s", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(param_count + 11));
                    }
                } else {
                    if (operation->arg1->type == P_TEMP) {
                        sprintf(name, "\tpush %s\t\t\t\t\t; Push function argument to the stack", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                    } else {
                        var_info *info = symbol_table_get(seg->table, operation->arg1->variable_name);
                        CG_var_address(info);
                        sprintf(name, "\tmov rax, qword[rax]\t\t\t\t; Load value of variable into rax\n\tpush rax\t\t\t\t\t; Push argument to the stack");
                    }
                }
                linked_list_append(CG_generated_code, name);
                param_count++;
                break;
            case IR_POP_PARAM:
                name = (char *) calloc(128, sizeof(char));
                if (param_count > 4) {
                    sprintf(name, "\tsub rsp, %d\t\t\t\t\t; Reset stack pointer after function call", operation->arg1->constant);
                    param_count = 4;
                } else {
                    if (param_count <= CG_current_frame->func_params) {
                        sprintf(name, "\tpop %s\t\t\t\t\t; Pop current function parameter %s from the stack", CG_reg_color_to_string(param_count + 10), CG_reg_color_to_string(param_count + 10));
                    }
                    param_count--;
                }
                linked_list_append(CG_generated_code, name);
                break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
                RA_node *arg1 = graph->nodes[operation->arg1->constant];
                RA_node *arg2 = graph->nodes[operation->arg2->constant];
                RA_node *arg3 = graph->nodes[operation->arg3->constant];
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov rax, %s\n", CG_reg_color_to_string((reg_color) arg2->color));
                linked_list_append(CG_generated_code, name);
                label = (char *) calloc(128, sizeof(char));
                if (code == IR_DIV) {
                    if (CG_current_frame->func_params > 3) {
                        linked_list_append(CG_generated_code, "\tpush rdx\t\t\t\t; Save value of function parameter\n");
                    }
                    sprintf(label, "\txor rdx, rdx\n%s %s\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color) arg3->color));
                } else {
                    if (operation->arg3->type == P_DEREFERENCE) {
                        sprintf(label, "%s rax, qword[%s]\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color) arg3->color));
                    } else {
                        sprintf(label, "%s rax, %s\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color) arg3->color));
                    }
                }
                linked_list_append(CG_generated_code, label);
                if (code == IR_DIV && CG_current_frame->func_params > 3) {
                    linked_list_append(CG_generated_code, "\tpop rdx\t\t\t\t\t; Restore value of function parameter\n");
                }
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov %s, rax", CG_reg_color_to_string((reg_color) arg1->color));
                linked_list_append(CG_generated_code, name);
                break;
            case IR_EQUALS:
            case IR_NEQUALS:
            case IR_LESS:
            case IR_GREATER:
            case IR_LESS_EQ:
            case IR_GREATER_EQ:
                arg1 = graph->nodes[operation->arg1->constant];
                arg2 = graph->nodes[operation->arg2->constant];
                arg3 = graph->nodes[operation->arg3->constant];
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tcmp %s, %s\n", CG_reg_color_to_string((reg_color) arg2->color), CG_reg_color_to_string((reg_color) arg3->color));
                linked_list_append(CG_generated_code, name);

                name = (char *) calloc(128, sizeof(char));
                label = IR_generate_label("false_rel", relational_counter++);
                label2 = IR_generate_label("end_rel", relational_counter++);
                sprintf(name, "%s %s\n", IR_decide_branching(operation), label);
                linked_list_append(CG_generated_code, name);

                // The true case, move 1 into result register and jump to end_rel
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov %s, 1\n\tjmp %s\n", CG_reg_color_to_string((reg_color) arg1->color), label2);
                linked_list_append(CG_generated_code, name);
                
                // The false case, move 0 into result register
                linked_list_append(CG_generated_code, label);
                linked_list_append(CG_generated_code, ":\n");
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov %s, 0\n%s:", CG_reg_color_to_string((reg_color) arg1->color), label2);
                linked_list_append(CG_generated_code, name);
                break;
            case IR_AND:
            case IR_OR:
                arg1 = graph->nodes[operation->arg1->constant];
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\ttest %s, %s\t\t\t\t; logical operation", CG_reg_color_to_string(arg1->color), CG_reg_color_to_string(arg1->color));
                linked_list_append(CG_generated_code, name);
                break;
            case IR_IF:
            case IR_WHILE:
                prev = (IR_operation *) lln->prev->data;
                if (prev->op < IR_EQUALS || IR_OR < prev->op) {
                    // Previous operation is not relational or logical
                    reg_color color = graph->nodes[prev->arg1->constant]->color;
                    name = (char *) calloc(128, sizeof(char));
                    sprintf(name, "\ttest %s, %s\t\t\t\t; Test value to set flags\n\tjz ", CG_reg_color_to_string(color), CG_reg_color_to_string(color));
                    linked_list_append(CG_generated_code, name);
                } else {
                    linked_list_append(CG_generated_code, IR_decide_branching(prev));
                }
                linked_list_append(CG_generated_code, (char *) operation->arg3->dest->name);
                break;
            case IR_ALLOC:
                arg1 = graph->nodes[operation->arg1->constant];
                arg2 = graph->nodes[operation->arg2->constant];
                arg3 = graph->nodes[operation->arg3->constant];
                linked_list_append(CG_generated_code, "\tpush rdi\n\tpush rsi\n");
                name = (char *) calloc(256, sizeof(char));
                sprintf(name, "\tmov rdi, %s\t\t\t\t; move size of elements into rdi\n\tmov rsi, %s\t\t\t\t; Move number of elements to be allocated into %s\n", CG_reg_color_to_string(arg2->color),CG_reg_color_to_string(arg3->color), CG_reg_color_to_string(arg3->color));
                linked_list_append(CG_generated_code, name);
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tcall _alloc\n\tpop rdi\n\tpop rsi\n\tmov %s, rax", CG_reg_color_to_string(arg1->color));
                linked_list_append(CG_generated_code, name);
                break;
            case IR_PRINT:
                int counter = 0;
                name = (char *) calloc(128, sizeof(char));
                while (counter < CG_current_frame->func_params && counter < 4) {
                    sprintf(name, "\tpush %s\t\t\t\t; Push function parameter %s to the stack (caller save)\n", CG_reg_color_to_string(counter + 11), CG_reg_color_to_string(counter + 11));
                    linked_list_append(CG_generated_code, name);
                    name = (char *) calloc(128, sizeof(char));
                    counter++;
                }
                if (operation->arg1->type == P_VARIABLE) {
                    var_info *info = symbol_table_get(operation->in_seg->table, operation->arg1->variable_name);
                    CG_var_address(info);
                    sprintf(name, "\tmov rdi, qword[rax]\t\t\t\t; Move value to be printed into rdi\n\tcall print_int\t\t\t\t; Call print_int");
                } else if (operation->arg1->type == P_DEREFERENCE) {
                    sprintf(name, "\tmov rdi, qword[%s]\t\t\t\t; Move value to be printed into rdi\n\tcall print_int\t\t\t\t; Call print_int", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                } else {
                    sprintf(name, "\tmov rdi, %s\t\t\t\t; Move value to be printed into rdi\n\tcall print_int\t\t\t\t; Call print_int", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                }
                linked_list_append(CG_generated_code, name);
                while (counter > 0) {
                    name = (char *) calloc(128, sizeof(char));
                    sprintf(name, "\n\tpop %s\t\t\t\t; Restore function parameter %s", CG_reg_color_to_string(counter + 10), CG_reg_color_to_string(counter + 10));
                    linked_list_append(CG_generated_code, name);
                    counter--;
                }
                break;
            case IR_CALL:
                name = (char *) calloc(128, sizeof(char));
                int i = param_count;
                //print_operation(operation);
                while (i < CG_current_frame->func_params && i < 4) {
                    sprintf(name, "\tpush %s\t\t\t\t; Push function parameter %s to the stack (caller save)\n", CG_reg_color_to_string(i + 11), CG_reg_color_to_string(i + 11));
                    linked_list_append(CG_generated_code, name);
                    name = (char *) calloc(128, sizeof(char));
                    i++;
                }
                CG_create_static_link(operation);
                sprintf(name, "\tcall %s\t\t\t\t; Call function\n\tadd rsp, 8\t\t\t\t; Yeet the static link\n", operation->arg2->call->name);
                linked_list_append(CG_generated_code, name);
                while (i > param_count) {
                    name = (char *) calloc(128, sizeof(char));
                    sprintf(name, "\tpop %s\t\t\t\t\t; Restore function parameter\n", CG_reg_color_to_string(i + 10));
                    linked_list_append(CG_generated_code, name);
                    i--;
                }
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov %s, rax\t\t\t\t; Move return value from rax", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                linked_list_append(CG_generated_code, name);
                break;
            case IR_RET:
                name = calloc(128, sizeof(char));
                if (operation->arg1) {
                    sprintf(name, "\tmov rax, %s\t\t\t\t; Move return value into rax\n", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                    linked_list_append(CG_generated_code, name);
                }
                name = calloc(128, sizeof(char));
                sprintf(name, "\tadd rsp, %d\n", CG_current_frame->max_offset - (CG_current_frame->regs_used * 8));
                linked_list_append(CG_generated_code, name);
                for (int i = 1; i <= CG_current_frame->regs_used; i++) {
                    name = (char *) calloc(128, sizeof(char));
                    sprintf(name, "\tpop %s\n", CG_reg_color_to_string(i));
                    linked_list_append(CG_generated_code, name);
                }
                linked_list_append(CG_generated_code, "\tmov rsp, rbp\t\t\t\t; Restore the old stack pointer before exit\n\tpop rbp\t\t\t\t\t\t; Restore the base pointer of the previous stack\n\tret\n");
                break;
            case IR_GOTO:
                // Whenever a goto is encountered, we straight up just return, works 10/10 times
                linked_list_append(CG_generated_code, "\tjmp ");
                linked_list_append(CG_generated_code, (char *) operation->arg1->dest->name);
                linked_list_append(CG_generated_code, "\n");
                return;
            case IR_LOGICAL_JUMP:
                IR_operation *prev = (IR_operation *) lln->prev->data;
                name = (char *) calloc(128, sizeof(char));
                if (prev->op ==IR_ASSIGN) {
                    sprintf(name, "\tjmp %s\n", operation->arg1->dest->name);
                } else {
                    sprintf(name, "%s %s\n", IR_decide_branching(prev), operation->arg1->dest->name);
                }
                linked_list_append(CG_generated_code, name);
                recurse_segment(seg->left, graph);
                return;
            default:
                printf("ir_codegen.c::recurse_segment: Unknown op code\n");
                return;
        }
        linked_list_append(CG_generated_code, "\n");
    }
    recurse_segment(seg->left, graph);
    recurse_segment(seg->right, graph);
}

void CG_find_offset(segment *seg) {
    if (!seg) {
        return;
    }
    if (!seg->operations->size) {
        return;
    }
    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        IR_operation *op = (IR_operation *) lln->data;
        if (op->op == IR_GOTO) {
            return;
        }
        if (op->op != IR_VAR_DECL || op->arg1->type == P_TEMP) {
            continue;
        }
        char *name = op->arg1->variable_name;
        var_info *info = (var_info *) symbol_table_get(seg->table, name);
        if (info->kind == ID_FUNC_PARAM) {
            continue;
        }
        print_operation(op);
        
        info->offset = 8 * CG_offset;
        CG_offset++;
    }
    CG_find_offset(seg->left);
    CG_find_offset(seg->right);
    return;
}

void CG_regs_used_helper(segment *seg, RA_graph *graph) {
    if (!seg) {
        return;
    }
    if (!seg->operations || !seg->operations->size) {
        return;
    }
    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        IR_operation *op = (IR_operation *) lln->data;
        if (op->op == IR_GOTO || op->op == IR_RET) {
            return;
        }
        if (op->op == IR_VAR_DECL || op->arg1->type != P_TEMP) {
            continue;
        }
        int color = graph->nodes[op->arg1->constant]->color;
        if (color < MAX_REG + 1 && color > CG_reg_max) {
            CG_reg_max = color;
        }
    }
    CG_regs_used_helper(seg->left, graph);
    CG_regs_used_helper(seg->right, graph);
    return;
}

void CG_regs_used(frame *frm, RA_graph *graph) {
    frame *current_frame;
    linked_list *frames = linked_list_new();
    linked_list_append(frames, frm);
    while (frames->size) {
        current_frame = linked_list_pop_front(frames);
        CG_regs_used_helper(current_frame->segment, graph);
        //printf("color: %d\n", CG_reg_max);
        current_frame->regs_used = CG_reg_max;
        CG_reg_max = 0;
        for (linked_list_node *lln = current_frame->nested_frames->head; lln != NULL; lln = lln->next) {
            linked_list_append(frames, (frame *) lln->data);
        }
    }
}

void prologue(frame *frm) {
    char *str = (char *) calloc(70, sizeof(char));
    linked_list_append(CG_generated_code, frm->name);
    linked_list_append(CG_generated_code, ":\n\tpush rbp\t\t\t\t\t; Save the old base pointer\n\tmov rbp, rsp\t\t\t\t; Set up base pointer for new stack frame\n");
    for (int i = frm->regs_used; i > 0; i--) {
        char *push = (char *) calloc(70, sizeof(char));
        sprintf(push, "\tpush %s\n", CG_reg_color_to_string(i));
        linked_list_append(CG_generated_code, push);
    }
    sprintf(str, "\tsub rsp, %d\t\t\t\t\t; Add max frame offset to stack pointer\n", frm->max_offset - (frm->regs_used * 8));
    linked_list_append(CG_generated_code, str);
}

void CG_recurse_initialised_array(char* buffer, linked_list *values, int depth, int current_depth, AST_node* node) {
    if (current_depth == depth) {
        char *elem = (char *) calloc(10000, sizeof(char));
        for (linked_list_node *lln = values->head; lln != NULL; lln = lln->next) {
            if (((AST_node *) lln->data)->primary_expr.type == TYPE_INT) {
                sprintf(elem, "%d,", ((AST_node *)lln->data)->primary_expr.integer_value);
                strcat(buffer, elem);
            }
        }
        free(elem);
    } else {
        for (linked_list_node *lln = values->head; lln != NULL; lln = lln->next) {
            CG_recurse_initialised_array(buffer, (linked_list *) lln->data, depth, current_depth + 1, node);
        }
    }
}

void code_emit(linked_list *emitted_code, frame *program, RA_graph *graph) {
    for (linked_list_node *lln = program->data->head; lln != NULL; lln = lln->next) {
        AST_node *node = (AST_node *) lln->data;
        char *buffer = (char *)calloc(500000, sizeof(char));
        if (node->kind == A_PRIMARY_EXPR) {
            sprintf(buffer, "\t%s db \"%s\"\n", IR_generate_label("_string", CG_string_counter++), node->primary_expr.string.value);
            printf("%s", buffer);
            linked_list_append(data_section, buffer);
        } else {
            int total_length = 1;
            for (linked_list_node *lln = node->array_decl.sizes->head; lln != NULL; lln = lln->next) {
                total_length *= (int) ((AST_node*) lln->data)->primary_expr.integer_value;
            }
            sprintf(buffer, "\t%s dq %d,%d, ", IR_generate_label("_array", CG_array_counter++), IR_get_type_size(node->array_decl.type), total_length);

            CG_recurse_initialised_array(buffer, node->array_decl.values, node->array_decl.sizes->size, 1, node);
            strcat(buffer, "\n");
            printf("%s\n", buffer);
            linked_list_append(data_section, buffer);
        }
    }
    if (program->name) {
        if (strncmp("main", program->name, 4) == 0) {
            //linked_list_append(CG_generated_code, \
            //    "global _start\n_start:\n");        
        }
        prologue(program);
        CG_current_frame = program;
        recurse_segment(program->segment, graph);
    } else {
        CG_generated_code = module_level;
        CG_current_frame = program;
        recurse_segment(program->segment, graph);
        CG_generated_code = emitted_code;
    }


    CG_frame_depth++;
    for (linked_list_node *lln = program->nested_frames->head; lln != NULL; lln = lln->next) {
        CG_offset = ((frame *) lln->data)->regs_used;
        CG_find_offset(((frame *) lln->data)->segment);
        ((frame *) lln->data)->max_offset = CG_offset * 8;
        code_emit(emitted_code, (frame *) lln->data, graph);
    }
    CG_frame_depth--;

    return;
} 

void codegen(linked_list *ll, frame *program, RA_graph *graph) {
    module_level = linked_list_new();
    CG_generated_code = linked_list_new();
    data_section = ll;
    linked_list_append(module_level, \
                "global _start\n_start:\n\tlea rax, qword[heap]\t\t\t ; Move starting address of the heap into rax \n\tmov qword[heap_pointer], rax\t\t\t ; Set up the heap pointer to point at the start of the heap\n\tmov rbp, rsp\n");
    IR_create_print_macro();
    linked_list_append(data_section, \
                "section .bss\n\toutput resb 256\n\theap resq 1000000\n\theap_pointer resq 1\n");
    linked_list_append(data_section, \
                "section .data\n\ttable db '0123456789'\n\tnewline db 0xa\n");
    IR_create_print_int();
    IR_create_alloc();

    CG_regs_used(program, graph);

    /**
     * THIS ONLY WORKS FOR ONE MODULE
     * Ideally, the global scope holds all global variables from a single basepointer
     */
    CG_offset = ((frame *) program->nested_frames->head->data)->regs_used;
    if (CG_offset == 0) {
        CG_offset = 1;
    }
    CG_find_offset(((frame *) program->nested_frames->head->data)->segment);
    ((frame *) program->nested_frames->head->data)->max_offset = CG_offset * 8;
    char *stack_mov = (char *) calloc(128, sizeof(char));
    sprintf(stack_mov, "\tsub rsp, %d\t\t\t\t; Move the stackpointer beyond the global variables\n", ((frame *) program->nested_frames->head->data)->max_offset);
    linked_list_append(module_level, stack_mov);

    CG_frame_depth++;
    for (linked_list_node *lln = program->nested_frames->head; lln != NULL; lln = lln->next) {
        code_emit(CG_generated_code, (frame *) lln->data, graph);
    }
    CG_frame_depth--;
    linked_list_append(data_section, "section .text\n\n");
    ll->tail->next = CG_generated_code->head;
    CG_generated_code->head->prev = ll->tail;
    ll->tail = CG_generated_code->tail;
    ll->size += CG_generated_code->size;

    linked_list_append(module_level, "\tmov rax, 1\n\txor rbx, rbx\n\tint 0x80\n\n");
    ll->tail->next = module_level->head;
    module_level->head->prev = ll->tail;
    ll->tail = module_level->tail;
    ll->size += module_level->size;
    return;
}