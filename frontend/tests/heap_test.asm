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