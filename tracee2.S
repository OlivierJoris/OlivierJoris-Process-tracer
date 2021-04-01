.global main
.text
main:
    mov $4, %eax
    push %eax
    call rec
    pop %eax
    mov $1, %eax
    mov $0, %ebx
    int $0x80
rec:
    cmp $0, %eax
    je .if_true
    push %eax
    dec %eax
    call rec
    pop %eax
.if_true:
    call print
    ret
print:
    mov $4, %eax
    mov $1, %ebx
    mov $hello, %ecx
    mov $12, %edx
    int $0x80
    ret
.data
hello:
    .ascii "Hello world\n"