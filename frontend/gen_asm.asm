%macro sys_write 3
	mov rdi, %1
	mov rsi, %2
	mov rdx, %3
	mov rax,1
	syscall
%endmacro

section .bss
	output resb 256
section .data
	table db '0123456789'
	newline db 0xa
section .text

print_int:
	push rbp
	mov rbp, rsp
	push rdi
	mov rdi, output
	lea r10, [rsp-1]
	xor rcx, rcx
	mov byte[r10], 0xa
	dec r10
	inc rcx
	mov rax, qword[rbp-8]
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
	mov rsp, rbp
	pop rbp
	ret

global _start
_start:	mov rbp, rsp
	mov [rbp+16], rbp
	mov rax, qword[rbp+16]
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 500
	push rax
	mov rax, 200
	push rax
	mov rax, 99
	push rax
	mov rax, 20
	push rax
	lea rax, [rbp+16]
	mov rax, qword[rax]
	push rax
	call main0
	add rsp, 24
	mov rsp, rbp
	pop rbp
	mov rax, 1
	int 0x80

main0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	lea rax, [rbp]
	lea rax, [rbp+16]
	mov rax, qword[rax]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 30
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	jmp end_nest0
nest0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	lea rax, [rbp]
	lea rax, [rbp+16]
	mov rax, qword[rax]
	lea rax, [rax+16]
	mov rax, qword[rax]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	lea rax, [rbp+16]
	mov rax, qword[rax]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	lea rax, [rbp+16]
	mov rax, qword[rax]
	lea rax, [rax+16]
	mov rax, qword[rax]
	mov rax, qword[rax-16]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 10
	push rax
	mov rax, qword[rbp+24]		; Load function argument from above base pointer
	pop rbx
	add rax, rbx
	jmp epilogue0
epilogue0:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

end_nest0:
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp+16]
	mov rax, qword[rax]
	push rax
	call nest0
	add rsp, 16
	jmp epilogue1
epilogue1:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

