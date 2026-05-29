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
_start:
	mov rbp, rsp
	mov rsp, rbp
	call main0
	mov rax, 1
	int 0x80

main0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	lea rax, [rbp+16]			; Load the address containing the address of the static link for link traversal
	mov rax, qword[rax]			; Dereference rax to get the address of the static link
	push rax					; Pushing static link to stack
	call regs0				; Calling function
	add rsp, 8					; Reset stack pointer after call, getting rid of function arguments
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	jmp epilogue0
epilogue0:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

regs0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	mov rax, 0
	push rax
	mov rax, 0
	push rax
	mov rax, 1
	push rax
	mov rax, 1
	push rax
	mov rax, 1
	push rax
	mov rax, 1
	push rax
	mov rax, 1
	push rax
	mov rax, 1
	push rax
	mov rax, 0
	push rax
while0:
	mov rax, 1215752192
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	pop rbx
	cmp rax, rbx
	jge false0
	mov rax, 1
	jmp end_rel0
false0:
	mov rax, 0
end_rel0:
	jge end_while0
	mov rax, 0
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-24]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-32]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-40]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-48]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-56]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-64]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-72], rax
	lea rax, [rbp]
	mov rax, qword[rax-72]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-16]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-16], rax
	mov rax, 1
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-8], rax

	jmp while0
end_while0:
	lea rax, [rbp]
	mov rax, qword[rax-16]		; Load the value of a variable into rax
	jmp epilogue1
epilogue1:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

