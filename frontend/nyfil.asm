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
        mov rax, 1
        push rax
        mov rax, 3
        pop rbx
        add rax, rbx
        push rax
        mov rax, 2
        push rax
        mov rax, qword[rbp-8]
        push rax
        mov rax, qword[rbp-16]
        pop rbx
        cmp rax, rbx
        jge else
        mov rax, 48
        push rax
        mov rax, 2
        pop rbx
        add rax, rbx
        mov qword[rbp-24], rax
        jmp end_if
else:
        mov rax, 48
        push rax
        mov rax, 3
        pop rbx
        add rax, rbx
        mov qword[rbp-24], rax
end_if:
        mov rax, qword[rbp-24]
        mov rdi, rax
        push rax
        call print_int
        pop rax
        mov rsp, rbp
        pop rbp
        mov rax, 1
        int 0x80
