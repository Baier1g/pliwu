#include "ir_codegen.h"

linked_list *generated_code, *module_level;
segment *current_segment;
frame *current_frame;
int frame_depth = 0;
int epilogue_count = 0;
int offset_counter = 0;

char *CG_reg_color_to_string(reg_color reg) {
    switch (reg) {
        case R15:
            return "R15";
        case R14:
            return "R14";
        case R13:
            return "R13";
        case R12:
            return "R12";
        case R11:
            return "R11";
        case R10:
            return "R10";
        default:
            printf("ir_codegen.c::CG_reg_color_to_string: Undefined register\n");
            return NULL;
    }
}

void IR_create_print_macro(void) {
    linked_list_append(generated_code, \
    "%macro sys_write 3\n\tmov rdi, %1\n\tmov rsi, %2\n\tmov rdx, %3\n\tmov rax,1\n\tsyscall\n%endmacro\n\n");
}

void IR_create_print_int(void) {
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

CG_op_code IR_op_code_to_CG(IR_op_code code) {
    switch (code) {
        case IR_ADD:
            return CG_ADD;
        case IR_SUB:
            return CG_SUB;
        case IR_MUL:
            return CG_MULT;
        case IR_DIV:
            return CG_DIV;
        case IR_GOTO:
            return CG_JMP;
        default:
            printf("ir_codegen.c::IR_op_code_to_CG: Unknown IR_op_code\n");
            return NULL;
    }
}

void CG_var_static_link_traversal(var_info *var, char *op) {
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
    current_segment = seg;

    for (linked_list_node *lln = seg->operations->head; lln != NULL; lln = lln->next) {
        IR_operation *operation = (IR_operation *) lln->data;
        IR_operation *prev;
        IR_op_code code = operation->op;
        char *name, *label;
        switch (code) {
            case IR_VAR_DECL:
                name = operation->arg1->variable_name;

                break;
            case IR_ASSIGN:
                name = (char *) calloc(64, sizeof(char));
                if (operation->arg1->type == P_TEMP) {
                    reg_color reg = (reg_color) graph->nodes[operation->arg1->constant]->color;
                    if (operation->arg2->type == P_CONSTANT) {
                        sprintf(name, "\tmov %s, %d", CG_reg_color_to_string(reg), operation->arg2->constant);
                    }

                    linked_list_append(generated_code, name);
                } else {
                    char *tmp = operation->arg1->variable_name;
                    var_info *var = (var_info *) symbol_table_get(seg->table, tmp);
                    
                }

                break;
            case IR_PARAM:
                break;
            case IR_POP_PARAM:
                break;
            case IR_ADD:

                break;
            case IR_SUB:
                break;
            case IR_MUL:
                break;
            case IR_DIV:
                break;
            case IR_EQUALS:
                break;
            case IR_NEQUALS:
                break;
            case IR_LESS:
                break;
            case IR_GREATER:
                break;
            case IR_LESS_EQ:
                break;
            case IR_GREATER_EQ:
                break;
            case IR_AND:
                break;
            case IR_OR:
                break;
            case IR_IF:
            case IR_WHILE:
                prev = (IR_operation *) lln->prev->data;
                linked_list_append(generated_code, IR_decide_branching(prev->op));
                linked_list_append(generated_code, (char *) operation->arg3->dest->name);
                break;
            case IR_PRINT:
                break;
            case IR_CALL:

                break;
            case IR_RET:
                linked_list_append(generated_code, "\n");
                break;
            case IR_GOTO:
                // Whenever a goto is encountered, we straight up just return, works 10/10 times
                linked_list_append(generated_code, "\tjmp ");
                linked_list_append(generated_code, (char *) operation->arg1);
                return;
            default:
                printf("ir_codegen.c::CG_create_operation: Unknown op code\n");
                return;
        }
        linked_list_append(generated_code, "\n");
    }
    recurse_segment(seg->left, graph);
    recurse_segment(seg->right, graph);
}

void code_emit(linked_list *emitted_code, frame *program, RA_graph *graph) {
    if (program->name) {
        if (strncmp("main", program->name, 4) == 0) {
        } else {
            linked_list_append(generated_code, program->name);
            linked_list_append(generated_code, ":\n\tpush rbp\t\t\t\t\t; Save the old base pointer\n\tmov rbp, rsp\t\t\t\t; Set up base pointer for new stack frame\n");
            recurse_segment(program->segment, graph);
            linked_list_append(generated_code, IR_generate_label("epilogue", epilogue_count++));
            linked_list_append(generated_code, ":\n\tmov rsp, rbp\t\t\t\t; Restore the old stack pointer before exit\n\tpop rbp\t\t\t\t\t\t; Restore the base pointer of the previous stack\n\tret\n\n");
        }
    } else {
        generated_code = module_level;
        recurse_segment(program->segment, graph);
        generated_code = emitted_code;
    }


    frame_depth++;
    for (linked_list_node *lln = program->nested_frames->head; lln != NULL; lln = lln->next) {
        code_emit(emitted_code, (frame *) lln->data, graph);
    }
    frame_depth--;

    return;
} 

void codegen(linked_list *ll, frame *program, RA_graph *graph) {
    module_level = linked_list_new();
    linked_list_append(module_level, \
                "global _start\n_start:\n\tmov rbp, rsp\n");
    generated_code = ll;
    IR_create_print_macro();
    linked_list_append(generated_code, \
                "section .bss\n\toutput resb 256\n");
    linked_list_append(generated_code, \
                "section .data\n\ttable db '0123456789'\n\tnewline db 0xa\nsection .text\n\n");
    IR_create_print_int();

    for (linked_list_node *lln = program->nested_frames->head; lln != NULL; lln = lln->next) {
        code_emit(ll, (frame *) lln->data, graph);
    }
    linked_list_append(module_level, "\tcall main\n\tmov rax, 1\n\tsyscall 0x80\n\n");
    ll->tail->next = module_level->head;
    module_level->head->prev = ll->tail;
    ll->size += module_level->size;
    return;
}