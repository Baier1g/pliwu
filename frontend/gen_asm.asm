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
	mov rax, 500
	push rax
	mov rax, 99
	push rax
	mov rax, 5
	push rax
	lea rax, [rbp+16]
	push rax
	call main0
	add rsp, 24
	mov rsp, rbp
	pop rbp
	mov rax, 1
	int 0x80

id0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	mov rax, 10
	push rax
	mov rax, 5
	pop rbx
	cmp rax, rbx
	jle false0
	mov rax, 1
	jmp end_rel0
false0:
	mov rax, 0
end_rel0:
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, qword[rbp+24]		; Load function argument from above base pointer
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, qword[rbp+32]		; Load function argument from above base pointer
	push rax
	mov rax, qword[rbp+24]		; Load function argument from above base pointer
	pop rbx
	cmp rax, rbx
	jne false1
	mov rax, 1
	jmp end_rel1
false1:
	mov rax, 0
end_rel1:
	jne else0
	mov rax, 0
	mov qword[rbp-8], rax
	jmp end_if0
else0:
	mov rax, 1
	mov qword[rbp-8], rax
end_if0:
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	jmp epilogue0
epilogue0:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

main0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	lea rax, [rbp+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	sub qword[rax], 16
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 30
	push rax
	mov rax, 102
	push rax
	mov rax, 50
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp+16]
	push rax
	call id0
	add rsp, 32
	push rax
	jmp end_nest0
nest0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	lea rax, [rbp+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	sub qword[rax], 16
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	lea rax, [rbp+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	sub qword[rax], 16
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	jmp end_nest_nest0
nest_nest0:
	push rbp					; Save the old base pointer
	mov rbp, rsp				; Set up base pointer for new stack frame
	lea rax, [rbp+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	lea rax, qword[rax+16]
	sub qword[rax], 16
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 10
	push rax
	mov rax, qword[rbp+24]		; Load function argument from above base pointer
	pop rbx
	add rax, rbx
	jmp epilogue1
epilogue1:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

end_nest_nest0:
	mov rax, 10
	push rax
	mov rax, qword[rbp+24]		; Load function argument from above base pointer
	push rax
	lea rax, [rbp+16]
	push rax
	call nest_nest0
	add rsp, 16
	pop rbx
	add rax, rbx
	jmp epilogue2
epilogue2:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

end_nest0:
	lea rax, [rbp]
	mov rax, qword[rax-8]		; Load the value of a variable into rax
	push rax
	lea rax, [rbp+16]
	push rax
	call nest0
	add rsp, 16
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	mov rax, 50000
	push rax
	mov rax, 10
	push rax
	mov rax, 20
	push rax
	mov rax, 1000
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-32]		; Load the value of a variable into rax
	pop rbx
	cmp rax, rbx
	jge false2
	mov rax, 1
	jmp end_rel2
false2:
	mov rax, 0
end_rel2:
	jge else2
	mov rax, 107
	push rax
	mov rax, 5
	push rax
	mov rax, 300
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-32]		; Load the value of a variable into rax
	pop rbx
	imul rax, rbx
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-24]		; Load the value of a variable into rax
	pop rbx
	sub rax, rbx
	mov qword[rbp-56], rax
	jmp end_if2
else2:
	mov rax, 200
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-32]		; Load the value of a variable into rax
	pop rbx
	imul rax, rbx
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-24]		; Load the value of a variable into rax
	pop rbx
	add rax, rbx
	mov qword[rbp-56], rax
	mov rax, 70
	push rax
	mov rax, 10
	push rax
	mov rax, 50
	pop rbx
	add rax, rbx
	pop rbx
	cmp rax, rbx
	jle false4
	mov rax, 1
	jmp end_rel4
false4:
	mov rax, 0
end_rel4:
	cmp rax, 0
	jne false3
	mov rax, 3
	push rax
	mov rax, 4
	push rax
	mov rax, 10
	push rax
	mov rax, 10
	pop rbx
	add rax, rbx
	pop rbx
	add rax, rbx
	pop rbx
	cmp rax, rbx
	jl false5
	mov rax, 1
	jmp end_rel5
false5:
	mov rax, 0
end_rel5:
	cmp rax, 0
	jne false3
	mov rax, 0
	jmp end_logical3
false3:
	mov rax, 1
end_logical3:
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-48]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	lea rax, [rbp]
	mov rax, qword[rax-48]		; Load the value of a variable into rax
	cmp rax, 0
	je false6
	mov rax, 0
	cmp rax, 0
	je false6
	mov rax, 1
	jmp end_logical6
false6:
	mov rax, 0
end_logical6:
	push rax
	lea rax, [rbp]
	mov rax, qword[rax-56]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
end_if2:
	lea rax, [rbp]
	mov rax, qword[rax-56]		; Load the value of a variable into rax
	mov rdi, rax				; Move argument to be printed from rax to rdi
	push rax					; Save value to be printed to the stack
	call print_int				; Call the print function
	pop rax						; Restore the printed value
	jmp epilogue3
epilogue3:
	mov rsp, rbp				; Restore the old stack pointer before exit
	pop rbp						; Restore the base pointer of the previous stack
	ret

