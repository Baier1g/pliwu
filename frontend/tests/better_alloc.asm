_better_alloc:
	push rbp						;
	mov rbp, rsp					;
	push r8							;
	push r9							; PROLOGUE
	push r10						;
	push r11						;
	mov r8, qword[heap_pointer] 	; Move base address of new array to be allocated into r8
	push r8							; Save the base address on the stack
	lea r10, [rbp + 16]				; The address of the first stack argument, i.e the size of this array
	mov r9, qword[r10]				; Move the size of this array into r9
	mov qword[r8], rdi				; Move element size onto the heap at the base address
	add r8, 8						; Increment r8 by 8 to get the address of the next quadword
	mov qword[r8], rsi				; Move dimensionality of this array onto the heap at base_addres + 8
	add r8, 8						; Increment r8 by 8 to get the address of the next quadword
	mov qword[r8], r9				; Move number of elements in this array onto the heap at base_address + 16
	add r8, 8						; Increment r8 by 8 to get the address of the next quadword
	mov qword[r8], 1				; Move 1 onto the heap at base_address + 24. This is the reference counter
	imul r9, rdi					; Multiply the number of elements by the element size to get the actual amount of space needed
	add r8, 8						; Increment r8 by 8 to get the address of the first element of the array
	lea r10, [r8 + r9]				; Calculate the address of the next free space on the heap, which is base_address + 32 + array_size
	mov qword[heap_pointer], r10	; Move the newly calculated address into the heap_pointer to make it point at the new first free space
	cmp rsi, 1						; Check the dimensionality of the array
	je _end_alloc					; If it is 1, this array has no subarrays and base_address can be returned
	mov rbx, 0						; Move 0 into rbx, as it will be used as counter
	sub rsi, 1						; Decrement dimensionality for sub-array allocation calls
	mov rax, 8						; Move 8 into rax
	imul rax, rsi					; Multiply rax by the dimensionality to get offset of the last stack variable from the first
	add rax, 16						; Add 16 to rax to get the actual offset of the last stack variable from the base pointer
	mov r9, rax						; Move the offset of the next stack variable into r9
_get_stack_variables:
	lea r10, [rbp + r9]				; Get the address of a stack variable
	mov r11, qword[r10]				; Move stack variable into r11
	push r11						; Push stack variable
	add rbx, 1						; Increment counter
	sub r9, 8						; Decrement offset
	cmp rbx, rsi					; Compare counter to dimensionality
	jne _get_stack_variables		; If not equal, loop to load the rest of the stack variables
_allocate_sub_arrays:
	call _better_alloc				; Call alloc recursively, element size is the same and dimensionality has already been decremented
	mov qword[r8], rax				; Move address of allocated array onto the heap
	add r8, rdi						; Increment r8 by the element size to point it at the next element
	cmp r8, qword[heap_pointer]		; Compare r8 to the heap_pointer
	jne _allocate_sub_arrays		; If not equal, more subarrays need to be allocated
	mov r9, rsi						; Move dimensionality into r9
	imul r9, 8						; Multiply r9 by 8 to get the space the stack variables use on the stack
	sub rsp, r9						; Decrement rsp to reset the stack pointer
_end_alloc:
	pop rax							; Restore base_address to rax
	pop r11							;
	pop r10							;
	pop r9							;
	pop r8							; EPILOGUE
	mov rsp, rbp					;
	pop rbp							;
	ret								;