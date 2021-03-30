#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>

void print(){
    syscall(SYS_write, 1, "Hello world\n", 12);
    return;
}

int main(void){
    print();
    exit(1);
}