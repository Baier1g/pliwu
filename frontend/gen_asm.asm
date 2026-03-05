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
	push rbp
	mov rbp, rsp
	mov rax, 108
	push rax
	mov rax, 1
	push rax
	call main0
	add rsp, 16
	mov rsp, rbp
	pop rbp
	mov rax, 1
	int 0x80

factorial0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	mov rax, 1
	push rax
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	pop rbx
	cmp rax, rbx
	jg false0
	mov rax, 1
	jmp end_rel0
false0:
	mov rax, 0
end_rel0:
	jg end_if0
	mov rax, 1
	jmp epilogue0
	jmp end_if0
end_if0:
	mov rax, 1
	push rax
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	pop rbx
	sub rax, rbx
	push rax
	call factorial0
	add rsp, 8
	push rax
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	pop rbx
	imul rax, rbx
	jmp epilogue0
epilogue0:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

main0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	mov rax, 24
	push rax
	call factorial_helper0
	add rsp, 8
	jmp epilogue1
epilogue1:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

factorial_helper0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	mov rax, 0
	push rax
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	pop rbx
	cmp rax, rbx
	jne false1
	mov rax, 1
	jmp end_rel1
false1:
	mov rax, 0
end_rel1:
	jne end_if2
	jmp epilogue2
	jmp end_if2
end_if2:
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	push rax
	call factorial0
	add rsp, 8
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 1
	push rax
	mov rax, qword[rbp+16]		; Load function argument from above base pointer
	pop rbx
	sub rax, rbx
	push rax
	call factorial_helper0
	add rsp, 8
	jmp epilogue2
epilogue2:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

