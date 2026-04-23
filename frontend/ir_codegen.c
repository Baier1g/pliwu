#include "ir_codegen.h"

linked_list *CG_generated_code, *module_level;
segment *CG_current_segment;
frame *CG_current_frame;
int CG_offset = 1;
int CG_reg_max = 0;
int CG_frame_depth = 0;
int relational_counter = 0;
int logical_counter = 0;
int epilogue_count = 0;
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
    linked_list_append(CG_generated_code, \
    "%macro sys_write 3\n\tmov rdi, %1\n\tmov rsi, %2\n\tmov rdx, %3\n\tmov rax,1\n\tsyscall\n%endmacro\n\n");
}

void IR_create_print_int(void) {
    // Print prelude
    linked_list_append(CG_generated_code, \
    "print_int:\n\tpush rbp\n\tmov rbp, rsp\n\tpush rdi\n\tmov rdi, output\n\tlea r10, [rsp-1]\n\txor rcx, rcx\n");
    // prelude p2: newline and setup
    linked_list_append(CG_generated_code, \
    "\tmov byte[r10], 0xa\n\tdec r10\n\tinc rcx\n\tmov rax, qword[rbp-8]\n\tmov r8, 10\n");
    // Processing the integer
    linked_list_append(CG_generated_code, \
    "L1:\n\txor rdx, rdx\n\tdiv r8\n\tmov r9b, [table+rdx]\n\tmov [r10], r9b\n\tdec r10\n\tinc rcx\n\ttest rax, rax\n\tjnz L1\n");
    // Printing the integer
    linked_list_append(CG_generated_code, \
    "\tlea rsi, [r10+1]\n\tcld\n_L1:\n\tmovsb\n\tcmp rsi, rsp\n\tjne _L1\n\tpop rdi\n\tsys_write 1, output, rcx\n");
    // Epilogue
    linked_list_append(CG_generated_code, \
    "\tmov rsp, rbp\n\tpop rbp\n\tret\n\n");
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
            sprintf(buffer, "\tmov rax, qword[rbp-%d]\t\t; Load the value of a variable into rax\n", var_offset);
        } else {
            sprintf(buffer, "\tmov rax, qword[rax-%d]\t\t; Load the value of a variable into rax\n", var_offset);
        }
    } else {
        // Function parameter
        var_offset = -(var_offset);
        sprintf(buffer, "\tmov rax, qword[rbp+%d]\t\t; Load function argument from above base pointer\n", var_offset);
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

void recurse_segment(segment *seg, RA_graph *graph) {
    if (!seg) {
        return;
    }
    printf("recursing segment\n");
    CG_current_segment = seg;
    if (seg->name) {
        linked_list_append(CG_generated_code, seg->name);
        linked_list_append(CG_generated_code, ":\n");
    }
    var_info *info;
    int param_count = 0;

    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        IR_operation *operation = (IR_operation *) lln->data;
        IR_operation *prev;
        IR_op_code code = operation->op;
        //printf("op_code: %s", IR_op_code_to_string(code));
        char *name, *label, *label2;
        switch (code) {
            case IR_VAR_DECL:
                if (operation->arg1->type == P_TEMP) {
                    continue;
                }
                name = operation->arg1->variable_name;
                label = (char *) calloc(256, sizeof(char));
                info = (var_info *) symbol_table_get(seg->table, name);
                sprintf(label, "\tmov [rbp-%d], %d\t\t\t\t; Assign constant to variable %s\n", info->offset, operation->arg2->constant, name);
                linked_list_append(CG_generated_code, label);
                break;
            case IR_ASSIGN:
                name = (char *) calloc(128, sizeof(char));
                if (operation->arg1->type == P_TEMP) {
                    reg_color reg = (reg_color) graph->nodes[operation->arg1->constant]->color;
                    if (operation->arg2->type == P_CONSTANT) {
                        sprintf(name, "\tmov %s, %d\t\t\t\t; Put constant value into register", CG_reg_color_to_string(reg), operation->arg2->constant);
                    } else if (operation->arg2->type == P_TEMP) {
                        sprintf(name, "\tmov %s, %s\t\t\t\t; Assign from register to register", CG_reg_color_to_string(reg), CG_reg_color_to_string(graph->nodes[operation->arg2->constant]->color));
                    } else {
                        var_info *var = (var_info *) symbol_table_get(seg->table, operation->arg2->variable_name);
                        CG_var_address(var);
                        sprintf(name, "\tmov %s, qword[rax]\t\t\t; Load value of variable into register", CG_reg_color_to_string(reg));
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
                    sprintf(name, "\tmov %s, %s\t\t\t\t; Move function argument into %s", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color), CG_reg_color_to_string(param_count + 11));
                } else {
                    sprintf(name, "\tpush %s\t\t\t\t\t; Push function argument to the stack", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
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
                    sprintf(label, "%s rax, %s\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color) arg3->color));
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
                label = IR_generate_label("false_rel", relational_counter);
                label2 = IR_generate_label("end_rel", relational_counter++);
                sprintf(name, "\t%s %s\n", IR_decide_branching(operation), label);
                linked_list_append(CG_generated_code, name);

                // The true case, move 1 into result register and jump to end_rel
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov %s, 1\n\tjmp %s", CG_reg_color_to_string((reg_color) arg1->color), label2);
                
                // The false case, move 0 into result register
                linked_list_append(CG_generated_code, label);
                linked_list_append(CG_generated_code, ":\n");
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tmov %s, 0\n%s:", CG_reg_color_to_string((reg_color) arg1->color), label2);
                linked_list_append(CG_generated_code, name);
                break;
            case IR_AND:
                break;
            case IR_OR:
                break;
            case IR_IF:
            case IR_WHILE:
                prev = (IR_operation *) lln->prev->data;
                linked_list_append(CG_generated_code, IR_decide_branching(prev));
                linked_list_append(CG_generated_code, (char *) operation->arg3->dest->name);
                break;
            case IR_PRINT:
                break;
            case IR_CALL:
                name = (char *) calloc(128, sizeof(char));
                int i = param_count;
                print_operation(operation);
                while (i < CG_current_frame->func_params && i < 4) {
                    sprintf(name, "\tpush %s\t\t\t\t; Push function parameter %s to the stack (caller save)\n", CG_reg_color_to_string(i + 11), CG_reg_color_to_string(i + 11));
                    linked_list_append(CG_generated_code, name);
                    name = (char *) calloc(128, sizeof(char));
                    i++;
                }
                sprintf(name, "\tcall %s\t\t\t\t; Call function\n", operation->arg2->call->name);
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
                sprintf(name, "\tsub rsp, %d\n", CG_current_frame->max_offset - (CG_current_frame->regs_used * 8));
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
            default:
                printf("ir_codegen.c::CG_create_operation: Unknown op code\n");
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
    sprintf(str, "\tadd rsp, %d\t\t\t\t\t; Add max frame offset to stack pointer\n", frm->max_offset - (frm->regs_used * 8));
    linked_list_append(CG_generated_code, str);
}

void code_emit(linked_list *emitted_code, frame *program, RA_graph *graph) {
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
    CG_generated_code = ll;
    linked_list_append(module_level, \
                "global _start\n_start:\n"); 
    IR_create_print_macro();
    linked_list_append(CG_generated_code, \
                "section .bss\n\toutput resb 256\n");
    linked_list_append(CG_generated_code, \
                "section .data\n\ttable db '0123456789'\n\tnewline db 0xa\nsection .text\n\n");
    IR_create_print_int();

    CG_regs_used(program, graph);
    CG_frame_depth++;
    for (linked_list_node *lln = program->nested_frames->head; lln != NULL; lln = lln->next) {
        code_emit(ll, (frame *) lln->data, graph);
    }
    CG_frame_depth--;

    linked_list_append(module_level, "\tcall main\n\tmov rax, 1\n\tsyscall 0x80\n\n");
    ll->tail->next = module_level->head;
    module_level->head->prev = ll->tail;
    ll->size += module_level->size;
    for (linked_list_node *lln = ll->head; lln != NULL; lln = lln->next) {
        printf("%s", (char *) lln->data);
    }
    return;
}