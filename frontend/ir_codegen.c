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

void create_print_char_array(void) {
    linked_list_append(CG_generated_code,
"_print_char_array:\n\
	push rbp                            ;\n\
	mov rbp, rsp                        ;\n\
	push r8                             ;\n\
	push r9                             ; PROLOGUE\n\
	push r10                            ;\n\
	push r11                            ;\n\
	lea r9, [rdi + 16]                  ; Load the address of the length of the array\n\
	mov r11, qword[r9]                  ; Move the length of the array into r11\n\
	mov rdi, output                     ; Move address of output into rdi\n\
	lea r10, [rsp-1]                    ; Load the address of the first byte past rsp into r10 (this is the print pointer)\n\
	xor rcx, rcx                        ; Clear rcx\n\
	mov byte[r10], 0xa                  ; Move a newline onto the stack\n\
	dec r10                             ; Decrement print pointer\n\
	mov r9, qword[rbp-40]				; Move the basse address of the array to be printed into r9\n\
	lea rax, [r9 + 32]                  ; Load the address of the first element of the array into rax\n\
	mov r8, r11                         ; Move the length of the arrayu into r8\n\
	imul r8, 8                          ; Multiply length by 8 to get amount of bytes used for elements (since everything is quadwords)\n\
	add rax, r8                         ; Add total amount of bytes used in the array to rax, making rax point at the last element of the array\n\
	xor r9, r9                          ; Clear r9\n\
_char_L1:\n\
	mov r9b, byte[rax]                  ; Move a char element into r9\n\
	mov [r10], r9b                      ; Put char element on the stack\n\
	;cmp r9b, 0                           This is to avoid printing null characters, might be useful in the future\n\
	;je _no_increment\n\
	inc rcx                             ; Increment counter\n\
_no_increment:\n\
	dec r10                             ; Move print pointer to next empty space\n\
	sub rax, 8                          ; Subtract 8 from rax to get the next element of the array\n\
	cmp rcx, r11                        ; Check if the entire array has been put on the stack\n\
	jle _char_L1                        ; Keep putting elements on the stack if more still exist\n\
	lea rsi, [r10+1]                    ; Load address of last element added to the stack to rsi\n\
	cld                                 ; Clear direction flag to ensure movsb processes the stack in the correct order\n\
_char__L1:\n\
	movsb                               ; Move a byte from the address pointed to by rsi into output\n\
	cmp rsi, rsp                        ; Repeat until all characters have been processed\n\
	jne _char__L1                       ;\n\
	inc rcx                             ; Increment rcx to include the newline character\n\
	sys_write 1, output, rcx            ; Write array to the terminal\n\
	pop r11                             ;\n\
	pop r10                             ;\n\
	pop r9                              ;\n\
	pop r8                              ; EPILOGUE\
	mov rsp, rbp                        ;\n\
	pop rbp                             ;\n\
	ret                                 ;\n\n");
}

void IR_create_print_char(void) {
    linked_list_append(CG_generated_code,
"print_char:\n\
	push rbp                            ;\n\
	mov rbp, rsp                        ; PROLOGUE\n\
    push r11                            ;\n\
	mov [output], rdi                   ; Move character to be printed into output\n\
	sys_write 1, output, 1              ; Write output to terminal\n\
    pop r11                             ;\n\
	mov rsp, rbp                        ; EPILOGUE\n\
	pop rbp                             ;\n\
	ret                                 ;\n\n");
}

void IR_create_print_string(void) {
    linked_list_append(CG_generated_code,
"_print_string:\n\
	push rbp                            ;\n\
	mov rbp, rsp                        ; PROLOGUE\n\
    push r11                            ;\n\
	xor rcx, rcx                        ; Clear rcx\n\
	mov rcx, qword[rbp+16]				; Move length of string to be printed into rcx\n\
	mov r11, rdi                        ; Move address of string to be printed into r11\n\
    dec rcx                             ; Decrement rcx to get rid of the null character\n\
	sys_write 1, r11, rcx               ; Write to input string terminal\n\
	mov r11, qword[newline]             ; Move a newline into r11\n\
	mov [output], r11\n                 ; Put newline in output\n\
	sys_write 1, output, 1              ; Print newline\n\
	pop r11                             ;\n\
	mov rsp, rbp                        ; EPILOGUE\n\
	pop rbp                             ;\n\
	ret                                 ;\n\n");
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
    linked_list_append(CG_generated_code,
"_better_alloc:\n\
	push rbp						;\n\
	mov rbp, rsp					;\n\
	push r8							;\n\
	push r9							; PROLOGUE\n\
	push r10						;\n\
	push r11						;\n\
	push r12                        ;\n\
    push rbx                        ;\n\
	mov r8, qword[heap_pointer] 	; Move base address of new array to be allocated into r8\n\
	mov r12, r8\n\
	push r8							; Save the base address on the stack\n\
	lea r10, [rbp + 16]				; The address of the first stack argument, i.e the size of this array\n\
	mov r9, qword[r10]				; Move the size of this array into r9\n\
	mov qword[r8], 8				; Move element size (EVERYTHING IS QUADWORDS) onto the heap at the base address\n\
	add r8, 8						; Increment r8 by 8 to get the address of the next quadword\n\
	mov qword[r8], rsi				; Move dimensionality of this array onto the heap at base_addres + 8\n\
	add r8, 8						; Increment r8 by 8 to get the address of the next quadword\n\
	mov qword[r8], r9				; Move number of elements in this array onto the heap at base_address + 16\n\
	add r8, 8						; Increment r8 by 8 to get the address of the next quadword\n\
	mov qword[r8], 1				; Move 1 onto the heap at base_address + 24. This is the reference counter\n\
	imul r9, 8						; Multiply the number of elements by the element size to get the actual amount of space needed (Everything is quadwords)\n\
	add r8, 8						; Increment r8 by 8 to get the address of the first element of the array\n\
	lea r10, [r8 + r9]				; Calculate the address of the next free space on the heap, which is base_address + 32 + array_size\n\
	mov qword[heap_pointer], r10	; Move the newly calculated address into the heap_pointer to make it point at the new first free space\n\
	cmp rsi, 1						; Check the dimensionality of the array\n\
	je _end_alloc					; If it is 1, this array has no subarrays and base_address can be returned\n\
	mov rbx, 0						; Move 0 into rbx, as it will be used as counter\n\
	sub rsi, 1						; Decrement dimensionality for sub-array allocation calls\n\
	mov rax, 8						; Move 8 into rax\n\
	imul rax, rsi					; Multiply rax by the dimensionality to get offset of the last stack variable from the first\n\
	add rax, 16						; Add 16 to rax to get the actual offset of the last stack variable from the base pointer\n\
	mov r9, rax						; Move the offset of the next stack variable into r9\n\
_get_stack_variables:\n\
	lea r10, [rbp + r9]				; Get the address of a stack variable\n\
	mov r11, qword[r10]				; Move stack variable into r11\n\
	push r11						; Push stack variable\n\
	add rbx, 1						; Increment counter\n\
	sub r9, 8						; Decrement offset\n\
	cmp rbx, rsi					; Compare counter to dimensionality\n\
	jne _get_stack_variables		; If not equal, loop to load the rest of the stack variables\n\
    mov rbx, qword[heap_pointer]	; Move the address of the heap pointer into rbx\n\
_allocate_sub_arrays:\n\
	call _better_alloc				; Call alloc recursively, element size is the same and dimensionality has already been decremented\n\
	mov qword[r8], rax				; Move address of allocated array onto the heap\n\
	add r8, 8						; Increment r8 by the element size to point it at the next element\n\
	cmp r8, rbx             		; Compare r8 to the heap_pointer\n\
	jl _allocate_sub_arrays			; If not equal, more subarrays need to be allocated\n\
	mov r9, rsi					    ; Move dimensionality into r9\n\
	imul r9, 8						; Multiply r9 by 8 to get the space the stack variables use on the stack\n\
	add rsp, r9					    ; Decrement rsp to reset the stack pointer\n\
_end_alloc:\n\
	pop rax							; Restore base_address to rax\n\
    pop rbx                         ;\n\
	pop r12                         ;\n\
	pop r11							;\n\
	pop r10							;\n\
	pop r9							; EPILOGUE\n\
	pop r8							;\n\
	mov rsp, rbp					;\n\
	pop rbp							;\n\
	ret								;\n\n");
}

void IR_create_init_array(void) {
    linked_list_append(CG_generated_code,
                       "; This function initializes and array.\n\
; RDI: The address of the array to initialize\n\
; RSI: The address of the data segment array holding the values\n\
_initialize_array:\n\
	push rbp						;\n\
	mov rbp, rsp					;\n\
	push r8							; PROLOGUE\n\
	push r9							;\n\
	push r10						;\n\
	push r11						;\n\
    push rbx                        ;\n\
	lea r8, [rdi + 8]				; Load address of dimensionality\n\
	mov r10, qword[r8]				; Get dimensionality of the array\n\
	lea r8, [rdi + 16]				; Load the address of the number of elements into r8\n\
	mov r9, qword[r8]				; Dereference r8 to get the number of elements and put them into r9\n\
	mov rbx, 0						; move 0 into rbx, it will be used as a counter\n\
	lea r8, [rdi + 32]				; Load address of the first subarray into r8\n\
	cmp r10, 1						; Check dimensionality against 1\n\
	je _values						; If dimensionality is 1, put values onto the heap\n\
	push rdi						; Save address of array\n\
_recurse_subarrays:\n\
	mov rdi, qword[r8]				; Load the base address of a subarray into rdi\n\
    push rbx                        ; Save value in rbx\n\
	call _initialize_array			; Recursively call _initialize_array to get to the proper depth\n\
    pop rbx                         ; Restore rbx\n\
	add r8, 8						; Add 8 to r8 to point it at the address of the next subarray\n\
	add rbx, 1						; Increment counter\n\
	cmp rbx, r9						; Check the counter against number of elements\n\
	jne _recurse_subarrays			; If not all subarrays have been initialised, loop\n\
	pop rdi							; Pop rdi to keep the stack balanced\n\
	jmp _end_init					; Jump to the epilogue\n\
_values:\n\
	mov r10, qword[rsi]				; Load a value from the data segment\n\
	mov qword[r8], r10				; Put loaded value onto the heap\n\
	;mov r11, qword[rdi]			; Get the element size of the array (everything is quadwords)\n\
	add r8, 8                       ; Increment r8 to point at the next heap address\n\
	add rsi, 8                      ; Increment rsi to get the address of the next element to be loaded\n\
	add rbx, 1                      ; Increment counter\n\
	cmp rbx, r9                     ; Compare counter to array length\n\
	jne _values                     ; Keep loading values while array length is not reached\n\
_end_init:\n\
    pop rbx                         ;\n\
	pop r11							;\n\
	pop r10							;\n\
	pop r9							;\n\
	pop r8							; EPILOGUE\n\
	mov rsp, rbp					;\n\
	pop rbp							;\n\
	ret								;\n\n");
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
    //printf("here?\n");
    if (seg->name) {
        linked_list_append(CG_generated_code, seg->name);
        linked_list_append(CG_generated_code, ":\n");
    }
    //printf("out\n");
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
                name = (char *) calloc(256, sizeof(char));
                //printf("henlo\n");
                if (operation->arg1->type == P_TEMP || operation->arg1->type == P_REFERENCE) {
                    reg_color reg = (reg_color) graph->nodes[operation->arg1->constant]->color;
                    char *op1_reg = CG_reg_color_to_string(reg);
                    operand_type type = operation->arg2->type;
                    char *op2_reg;
                    switch(type) {
                        case P_CONSTANT:
                            sprintf(name, "\tmov %s, %d\t\t\t\t; Put constant value into register", op1_reg, operation->arg2->constant);
                            break;
                        case P_TEMP:
                            op2_reg = CG_reg_color_to_string(graph->nodes[operation->arg2->constant]->color);
                            sprintf(name, "\tmov %s, %s\t\t\t\t; Put constant value into register", op1_reg, op2_reg);
                            break;
                        case P_REFERENCE:
                        case P_DEREFERENCE:
                            op2_reg = CG_reg_color_to_string(graph->nodes[operation->arg2->constant]->color);
                            sprintf(name, "\tmov %s, qword[%s]\t\t\t\t; Assign from memory to register", op1_reg, op2_reg);
                            break;
                        case P_VARIABLE:
                            var_info *var = (var_info *) symbol_table_get(seg->table, operation->arg2->variable_name);
                            if (operation->arg2->variable_name[0] == '_') {
                                //printf("Got here\n");
                                label = operation->arg2->variable_name;
                                sprintf(name, "\tlea %s, [%s]\t\t\t; Load starting address of string literal %s into %s\n", op1_reg, label, label, op1_reg);
                            } else {
                                CG_var_address(var);
                                sprintf(name, "\tmov %s, qword[rax]\t\t\t; Load value of variable into register", op1_reg);
                            }
                            break;
                        default:
                            printf("ir_codegen.c::recurse_segment::IR_ASSSIGN: Unkown operand type\n");
                        break;
                    }
                } else if (operation->arg1->type == P_DEREFERENCE) {
                    reg_color reg = (reg_color) graph->nodes[operation->arg1->constant]->color;
                    reg_color reg2 = (reg_color) graph->nodes[operation->arg2->constant]->color;
                    if (operation->arg2->type == P_DEREFERENCE) {
                        label = (char *) calloc(128, sizeof(char));
                        sprintf(label, "\tmov rax, qword[%s]\t\t\t\t; Move the value of the address in %s into rax\n", CG_reg_color_to_string(reg2), CG_reg_color_to_string(reg2));
                        linked_list_append(CG_generated_code, label);
                        sprintf(name, "\tmov qword[%s], rax\t\t\t\t; Move value of rax into memory address pointed to by %s", CG_reg_color_to_string(reg), CG_reg_color_to_string(reg));
                    } else if (operation->arg2->type == P_CONSTANT) {
                        sprintf(name, "\tmov qword[%s], %d\t\t\t\t; Move value of a constant into memory address pointed to by %s", CG_reg_color_to_string(reg), operation->arg2->constant, CG_reg_color_to_string(reg));
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
                    if (operation->arg2) {
                        sprintf(name, "\tpush %s\t\t\t\t\t; Push function argument to the stack", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                        linked_list_append(CG_generated_code, name);
                        break;
                    }
                    if (CG_current_frame->func_params && param_count <= CG_current_frame->func_params) {
                        sprintf(name, "\tpush %s\t\t\t\t; Push current function parameter %s to the stack\n", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(param_count + 11));
                        linked_list_append(CG_generated_code, name);
                        name = (char *) calloc(128, sizeof(char));
                    }
                    if (operation->arg1->type == P_TEMP) {
                        sprintf(name, "\tmov %s, %s\t\t\t\t; Move function argument into %s", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color), CG_reg_color_to_string(param_count + 11));
                    } else if (operation->arg1->type == P_CONSTANT) {
                        sprintf(name, "\tmov %s, %d\t\t\t\t; Move constant function argument into %s", CG_reg_color_to_string(param_count + 11), operation->arg1->constant, CG_reg_color_to_string(param_count + 11));
                    } else {
                        var_info *info = symbol_table_get(seg->table, operation->arg1->variable_name);
                        CG_var_address(info);
                        sprintf(name, "\tmov %s, qword[rax]\t\t\t\t; Move function argument into %s", CG_reg_color_to_string(param_count + 11), CG_reg_color_to_string(param_count + 11));
                    }
                } else {
                    if (operation->arg2) {
                        sprintf(name, "\tpush %s\t\t\t\t\t; Push function argument to the stack", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                        linked_list_append(CG_generated_code, name);
                        break;
                    } else if (operation->arg1->type == P_TEMP) {
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
                    sprintf(name, "\tadd rsp, %d\t\t\t\t\t; Reset stack pointer after function call", operation->arg1->constant);
                    param_count = 4;
                } else if (operation->arg2) {
                    sprintf(name, "\tadd rsp, %d\t\t\t\t\t; Reset stack pointer after alloc call", operation->arg1->constant);
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
                if (operation->arg2->type == P_DEREFERENCE) {
                    sprintf(name, "\tmov rax, qword[%s]\n", CG_reg_color_to_string((reg_color)arg2->color));
                } else {
                    sprintf(name, "\tmov rax, %s\n", CG_reg_color_to_string((reg_color) arg2->color));
                }
                linked_list_append(CG_generated_code, name);
                label = (char *) calloc(128, sizeof(char));
                if (code == IR_DIV) {
                    if (CG_current_frame->func_params > 3) {
                        linked_list_append(CG_generated_code, "\tpush rdx\t\t\t\t; Save value of function parameter\n");
                    }
                    if (operation->arg3->type == P_DEREFERENCE) {
                        sprintf(label, "\txor rdx, rdx\n%s qword[%s]\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color) arg3->color));
                    } else {
                        sprintf(label, "\txor rdx, rdx\n%s %s\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color)arg3->color));
                    }
                } else {
                    if (operation->arg3->type == P_DEREFERENCE) {
                        sprintf(label, "%s rax, qword[%s]\n", CG_IR_op_code_to_string(code), CG_reg_color_to_string((reg_color)arg3->color));
                    } else if (operation->arg3->type == P_CONSTANT) {
                        sprintf(label, "%s rax, %d\n", CG_IR_op_code_to_string(code), operation->arg3->constant);
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
                if (operation->arg2->type == P_DEREFERENCE || operation->arg2->type == P_REFERENCE) {
                    sprintf(name, "\tmov %s, qword[%s]\n", CG_reg_color_to_string((reg_color)arg2->color), CG_reg_color_to_string((reg_color)arg2->color));
                    linked_list_append(CG_generated_code, name);
                    name = (char *) calloc(128, sizeof(char));
                }
                if (operation->arg3->type == P_DEREFERENCE || operation->arg3->type == P_REFERENCE) {
                    sprintf(name, "\tmov %s, qword[%s]\n", CG_reg_color_to_string((reg_color)arg3->color), CG_reg_color_to_string((reg_color)arg3->color));
                    linked_list_append(CG_generated_code, name);
                    name = (char *) calloc(128, sizeof(char));
                }
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
                if (!lln->prev) {
                    reg_color color = graph->nodes[operation->arg1->constant]->color;
                    name = (char *) calloc(128, sizeof(char));
                    sprintf(name, "\tcmp %s, 0\n\tje ", CG_reg_color_to_string(color));
                    linked_list_append(CG_generated_code, name);
                } else {
                    prev = (IR_operation *) lln->prev->data;
                    reg_color color = graph->nodes[prev->arg1->constant]->color;
                    if (prev && prev->op < IR_EQUALS || IR_OR < prev->op) {
                        // Previous operation is not relational or logical
                        name = (char *) calloc(128, sizeof(char));
                        if (prev->arg1->type == P_REFERENCE) {
                            sprintf(name, "\tmov rax, qword[%s]\n\ttest qword[%s], rax\t\t\t\t; Test value to set flags\n\tjz ", CG_reg_color_to_string(color), CG_reg_color_to_string(color));
                        } else {
                            sprintf(name, "\ttest %s, %s\t\t\t\t; Test value to set flags\n\tjz ", CG_reg_color_to_string(color), CG_reg_color_to_string(color));
                        }
                    linked_list_append(CG_generated_code, name);
                    } else {
                        linked_list_append(CG_generated_code, IR_decide_branching(prev));
                    }
                }
                linked_list_append(CG_generated_code, (char *) operation->arg3->dest->name);
                break;
            case IR_ALLOC:
                arg1 = graph->nodes[operation->arg1->constant];
                name = (char *) calloc(128, sizeof(char));
                sprintf(name, "\tcall _better_alloc\n\tmov %s, rax", CG_reg_color_to_string(arg1->color));
                linked_list_append(CG_generated_code, name);
                break;
            case IR_INIT:
                linked_list_append(CG_generated_code, "\tcall _initialize_array");
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
                if (operation->arg1->type == P_VARIABLE && operation->arg2->constant != TYPE_STRING) {
                    var_info *info = symbol_table_get(operation->in_seg->table, operation->arg1->variable_name);
                    CG_var_address(info);
                    sprintf(name, "\tmov rdi, qword[rax]\t\t\t\t; Move value to be printed into rdi\n");
                } else if (operation->arg1->type == P_DEREFERENCE && operation->arg2->constant != TYPE_STRING) {
                    sprintf(name, "\tmov rdi, qword[%s]\t\t\t\t; Move value to be printed into rdi\n", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                } else if (operation->arg1->type == P_CONSTANT) {
                    sprintf(name, "\tmov rdi, %d\t\t\t\t; Move value to be printed into rdi\n", operation->arg1->constant);
                } else if (operation->arg1->type == P_TEMP && operation->arg2->constant != TYPE_STRING) {
                    sprintf(name, "\tmov rdi, %s\t\t\t\t; Move value to be printed into rdi\n", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                }
                linked_list_append(CG_generated_code, name);
                //printf("yurr\n");
                name = (char *) calloc(128, sizeof(char));
                if (operation->arg2 && operation->arg2->constant == TYPE_CHAR) {
                    if (operation->arg3->constant == 1) {
                        sprintf(name, "\tcall _print_char_array\t\t\t\t; Call _print_char_array");
                    } else {
                        sprintf(name, "\tcall print_char\t\t\t\t; Call print_char");
                    }
                } else if (operation->arg2 && operation->arg2->constant == TYPE_STRING) {
                    sprintf(name, "\tmov rax, qword[%s]\t\t\t; Load address of string length into rax\n\tpush rax\n", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                    linked_list_append(CG_generated_code, name);
                    name = (char *) calloc(128, sizeof(char));
                    sprintf(name, "\tadd %s, 8\n\tmov rdi, %s\n\tcall _print_string\n\tadd rsp, 8", CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color), CG_reg_color_to_string(graph->nodes[operation->arg1->constant]->color));
                } else {
                    sprintf(name, "\tcall print_int\t\t\t\t; Call print_int");
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
        //print_operation(op);
        
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
        if (op->op == IR_INIT || op->op == IR_VAR_DECL || op->arg1->type != P_TEMP) {
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
            } else if (((AST_node *) lln->data)->primary_expr.type == TYPE_CHAR) {
                sprintf(elem, "%d,", ((AST_node *)lln->data)->primary_expr.char_value);
            } else if (((AST_node *) lln->data)->primary_expr.type == TYPE_BOOL) {
                sprintf(elem, "%d,", ((AST_node *)lln->data)->primary_expr.bool_value);
            }
            strcat(buffer, elem);
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
        //printf("yurr\n");
        char *buffer = (char *) calloc(50000, sizeof(char));
        if (node->kind == A_PRIMARY_EXPR) {
            char *string_name = IR_generate_label("_string", CG_string_counter++);
            sprintf(buffer, "\t%s dq %d\n\t%s_elem db \"%s\"\n", string_name, node->primary_expr.string.length, string_name, node->primary_expr.string.value);
            //printf("%s", buffer);
            free(string_name);
            linked_list_append(data_section, buffer);
        } else {
            int total_length = 1;
            char* name = IR_generate_label("_array", CG_array_counter++);
            if (node->array_decl.type == TYPE_INT || node->array_decl.type == TYPE_STRING) {
                sprintf(buffer, "\t%s dq ", name);
            } else {
                sprintf(buffer, "\t%s db ", name);
            }
            CG_recurse_initialised_array(buffer, node->array_decl.values, node->array_decl.sizes->size, 1, node);
            strcat(buffer, "\n");
            //printf("%s\n", buffer);
            linked_list_append(data_section, buffer);
        }
        //printf("we made it\n");
    }
    if (program->name) {
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
    create_print_char_array();
    IR_create_print_char();
    IR_create_print_string();
    IR_create_alloc();
    IR_create_init_array();
    
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
    
    linked_list_append(module_level, "\tlea rax, [rbp]\n\tpush rax\n\tcall main\n\tmov rax, 1\n\txor rbx, rbx\n\tint 0x80\n\n");
    ll->tail->next = module_level->head;
    module_level->head->prev = ll->tail;
    ll->tail = module_level->tail;
    ll->size += module_level->size;
    return;
}