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
	mov rbx, 0      ; clear rbx
	mov rbx, rax    ; save number to be printed in rbx
	shl rax, 1      ; left shift rax to get msb in CF
	mov rax, rbx    ; restore number to be printed
	jnc prin        ; jump past negation if carry flag not set
	neg rax         ; negate rax
	mov r14, 1      ; move 1 into r14 to indicate a negative number
prin:
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
	cmp r14, 1          ; check if r14 is set
	jne printtt         ; jump past minus if it isn't
	mov r14, 0          ; reset r14 for following calls to print
	mov byte[r10], 0x2d ; move "-" to the output
	dec r10             ; decrement string pointer
	inc rcx             ; increase counter register
printtt:
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