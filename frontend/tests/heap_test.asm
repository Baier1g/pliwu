%macro sys_write 3
	mov rdi, %1
	mov rsi, %2
	mov rdx, %3
	mov rax,1
	syscall
%endmacro

section .bss
	output resb 256
	heap resq 1000000
	heap_pointer resq 1
section .data
	table db '0123456789'
	newline db 0xa
section .text

print_int:
	push rbp
	mov rbp, rsp
	push r8
	push r9
	push r10
	push rdi
	mov rdi, output
	lea r10, [rsp-1]
	xor rcx, rcx
	mov byte[r10], 0xa
	dec r10
	inc rcx
	mov rax, qword[rbp-32]
	mov r8, 10
L1:
	xor rdx, rdx
	div r8
	mov r9b, [table+rdx]
	mov [r10], r9b
	dec r10
	inc rcx
	test rax, rax
	jnz L1
	lea rsi, [r10+1]
	cld
_L1:
	movsb
	cmp rsi, rsp
	jne _L1
	pop rdi
	sys_write 1, output, rcx
	pop r10
	pop r9
	pop r8
	mov rsp, rbp
	pop rbp
	ret

global _start
_start:
	lea rax, [heap] 				;; Set up heap
	mov qword[heap_pointer], rax	;; Set heap_pointer to the base address of the heap
	mov rbp, rsp
	sub rsp, 16				        ; Move the stackpointer beyond the global variables
	lea rdi, heap					;; Load address of the heap into rdi
	call print_int					;; print heap address
	lea rax, heap_pointer			;; Load the address of the heap pointer into rdi
	mov rdi, rax
	call print_int					;; print heap_pointer address
	mov qword[heap_pointer], heap	;; Move the address of the heap directly into the heap pointer
	mov rdi, qword[heap_pointer]	; Dereference heap_pointer to get the address of the heap and move it into rdi
	call print_int					; print address of the heap
	
alloc_and_print:					; writes 10000 numbers to the heap and prints them
	mov r8, qword[heap_pointer]		; get the current address of the heap and mov it into r8
	mov r10, 0						; r10 will keep track of the offset from the heap pointer
	mov r9, 10000					; r9 will act as a counter, set to 10000
load_array:
	mov qword[r8+r10], r9			; move the value of r9 onto the heap at offset r10
	sub r9, 1						; decrement r9
	add r10, 8						; add 8 to r10 to keep track of the current offset
	cmp r9, 0						; check if loading is done yet
	jne load_array					; jump if r9 isn't 0
end_load:
	add qword[heap_pointer], r10	; increment heap pointer position by added elements * 8
	mov rdi, qword[heap_pointer]	; move the address heap_pointer is pointing to into rdi
	call print_int					; print rdi for verification
	mov r9, 10000					; intermediate step, r9 acts as counter once again
	mov r8, qword[heap_pointer]		; move the new heap pointer position into r8
	sub r8, 8						; since the heap pointer points at the next free space, subtract 8		
	mov r10, 0

print_loop:							; print loop for an array of integers in the opposite order of which they were added
	mov r11, qword[r8+r10]			; r8 points to the end of the array on the heap, r10 is an offset starting at 0
	mov rdi, r11					; move the value into rdi
	call print_int					; print the value
	sub r10, 8						; decrement r10 by eight
	cmp r10, -80008					; when r10 is past the amount of elements * 8 in the array by 8 bytes, printing is finished 
	jne print_loop

    mov rax, 1
	xor rbx, rbx
	int 0x80

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


_better_alloc:
	push rbp						;
	mov rbp, rsp					;
	push r8							;
	push r9							; PROLOGUE
	push r10						;
	push r11						;
	push r12
	mov r8, qword[heap_pointer] 	; Move base address of new array to be allocated into r8
	mov r12, r8
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
    mov rbx, qword[heap_pointer]	; Move the address of the heap pointer into rbx
_allocate_sub_arrays:
	call _better_alloc				; Call alloc recursively, element size is the same and dimensionality has already been decremented
	mov qword[r8], rax				; Move address of allocated array onto the heap
	add r8, 8						; Increment r8 by the element size to point it at the next element
	cmp r8, rbx             		; Compare r8 to the heap_pointer
	jne _allocate_sub_arrays		; If not equal, more subarrays need to be allocated
	lea r10, [r12 + 8]
	mov r11, qword[r10]
	imul r11, 8
	add rsp, r11
	mov r9, rsi						; Move dimensionality into r9
	imul r9, 8						; Multiply r9 by 8 to get the space the stack variables use on the stack
	sub rsp, r9						; Decrement rsp to reset the stack pointer
_end_alloc:
	pop rax							; Restore base_address to rax
	pop r12
	pop r11							;
	pop r10							;
	pop r9							;
	pop r8							; EPILOGUE
	mov rsp, rbp					;
	pop rbp							;
	ret								;