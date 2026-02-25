%macro sys_write 3
    mov rdi, %1
    mov rsi, %2
    mov rdx, %3
    mov eax,1
    syscall
%endmacro

section     .text

extern printf
global _start                              ; Entry point for the program
_start: 
    mov rdi, output
    lea r10, [rsp-1]
    mov byte[r10], 0xa
    dec r10
    mov rax, 5
    mov r9b, [table+rax]
    mov [r10], r9b
    dec r10
    mov rax, 4
    mov r9b, [table+rax]
    mov [r10], r9b
    dec r10
    lea rsi, [r10+1]
    cld
_L1:
    movsb
    cmp rsi,rsp
    jne _L1
    sys_write 1,output,3                                      ; Start of the program
    
    mov     rdx,len                            ; Calculate message length
    mov     rcx,msg                           ; Load address of message
    mov     rbx,1                               ; File descriptor (stdout)
    mov     rax,4                               ; System call number (sys_write)
    int     0x80                                ; Call kernel to display message

    mov     eax,1                               ; System call number (sys_exit)
    int     0x80                                ; Call kernel to exit program

section .bss
    output resb 256

section     .data
    table db '0123456789'

msg     db  'H',10                    ; Our message with a newline
len     equ $ - msg                             ; Calculate length of message