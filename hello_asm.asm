section .data
    msg db 'Hello, Assembly!', 0xA  ; 字符串，0xA是换行符
    len equ $-msg

section .text
    global _start

_start:
    mov rax, 1          ; 系统调用号1，write
    mov rdi, 1          ; 文件描述符1，stdout
    mov rsi, msg        ; 要输出的字符串地址
    mov rdx, len        ; 字符串长度
    syscall             ; 调用内核

    mov rax, 60         ; 系统调用号60，exit
    xor rdi, rdi        ; 返回值0
    syscall             ; 退出程序
