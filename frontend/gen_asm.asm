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
	mov rax, 99
	push rax
	mov rax, 5
	push rax
	call main0
	add rsp, 16
	mov rsp, rbp
	pop rbp
	mov rax, 1
	int 0x80

id0:
	push rbp
	mov rbp, rsp
	mov rax, 1
	push rax
	mov rax, qword[rbp-0]
	push rax
	mov rax, qword[rbp-0]
	pop rbx
	cmp rax, rbx
	jne else0
	mov rax, 0
	mov qword[rbp-8], rax
	jmp end_if0
else0:
	mov rax, 1
	mov qword[rbp-8], rax
end_if0:
	mov rax, qword[rbp-8]
	mov rsp, rbp
	pop rbp
	ret

main0:
	push rbp
	mov rbp, rsp
	mov rax, 30
	push rax
	mov rax, 102
	push rax
	mov rax, 30
	push rax
	mov rax, qword[rbp-8]
	push rax
	call id0
	add rsp, 24
	push rax
	mov rax, 50000
	push rax
	mov rax, 10
	push rax
	mov rax, 0
	push rax
	mov rax, 1000
	push rax
	mov rax, qword[rbp-32]
	pop rbx
	cmp rax, rbx
	jle else2
	mov rax, 107
	push rax
	mov rax, 50
	mov qword[rbp-40], rax
	jmp end_if2
else2:
	mov rax, 200
	mov qword[rbp-40], rax
	push rax
end_if2:
	mov rax, qword[rbp-40]
	mov rdi, rax
	push rax
	call print_int
	pop rax
	mov rsp, rbp
	pop rbp
	ret

