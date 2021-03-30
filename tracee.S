.global main
.text
main:
    call print
    mov $1, %eax
    mov $0, %ebx
    int $0x80
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