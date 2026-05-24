%macro sys_write 3
	mov rdi, %1
	mov rsi, %2
	mov rdx, %3
	mov rax,1
	syscall
%endmacro

section .bss
    output resb 1000
section .data
    hello db "Hello World!"
section .text

print_char:
	push rbp
	mov rbp, rsp
	push r8
	push r9
	push r10
	push r11
	push rdi
	mov rdi, output
	lea r10, [rsp-1]
	mov rax, qword[rbp-40]				; Access provided argument on the stack
	mov qword[r10], rax
	lea rsi, [r10+1]
	cld
_L1:
	movsb
	cmp rsi, rsp
	jne _L1
	pop rdi
	sys_write 1, output, rcx
	pop r11
	pop r10
	pop r9
	pop r8
	mov rsp, rbp
	pop rbp
	ret

_print_char_array:
	push rbp
	mov rbp, rsp
	push r8
	push r9
	push r10
	push r11
	lea r9, [rdi + 16]
	mov r11, qword[r9]
	push rdi
	mov rdi, output
	lea r10, [rsp-1]
	xor rcx, rcx
	mov byte[r10], 0xa
	dec r10
	mov r9, qword[rbp-40]				; Access provided argument on the stack
	lea rax, [r9 + 32]
	mov r8, r11
	imul r8, 8
	add rax, r8
	xor r9, r9
_char_L1:
	mov r9b, byte[rax]
	mov [r10], r9b
	;cmp r9b, 0
	;je _no_increment
	inc rcx
_no_increment:
	dec r10
	sub rax, 8
	cmp rcx, r11
	jle _char_L1
	lea rsi, [r10+1]
	cld
_char__L1:
	movsb
	cmp rsi, rsp
	jne _char__L1
	inc rcx
	pop rdi
	sys_write 1, output, rcx
	pop r11
	pop r10
	pop r9
	pop r8
	mov rsp, rbp
	pop rbp
	ret

    
global _start
_start:
    mov rbp, rsp
    mov r8, hello
    mov rdi, hello
    jmp print_char