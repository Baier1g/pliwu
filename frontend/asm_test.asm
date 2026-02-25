%macro sys_write 3
    mov rdi, %1
    mov rsi, %2
    mov rdx, %3
    mov rax,1
    syscall
%endmacro

section .text

global _start
_start:
    mov     rax, 2
    push    rax
    mov     rax, 1
    pop     rbx
    cmp     rax, rbx
jge else
    mov rax, 1
    push    rax
    mov     rax, 2
    pop     rbx
    add     rax, rbx
jmp end_if
else:
    mov rax, 1
    push    rax
    mov rax, 3
    pop rbx
    add rax, rbx
end_if:
    mov rdi, output
    lea r10, [rsp-1]
    xor rcx, rcx
    mov byte[r10], 0xa
    dec r10
    inc rcx
    mov r8,10
L1:
    xor rdx,rdx
    div r8
    mov r9b, [table+rdx]
    mov [r10],r9b
    dec r10
    inc rcx
    test rax,rax
    jnz L1
    lea rsi, [r10+1]
    cld
_L1:
    movsb
    cmp rsi,rsp
    jne _L1
    sys_write 1,output,rcx
    
    mov rax,1
    int 0x80

section .bss
    output resb 256

section .data
    table db '0123456789'
    newline db 0xa

