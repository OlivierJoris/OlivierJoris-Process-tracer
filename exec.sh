#!/bin/bash

echo -e "\n** tracee1.S **\n"
gcc tracee.S -g -o tracee --static -nostdlib -Wl,-entry="main"
./tracer -p tracee

echo -e "\n** tracee2.s **\n"
gcc tracee2.S -g -o tracee --static -nostdlib -Wl,-entry="main"
./tracer -p tracee

echo -e "\n** tracee1.c **\n"
gcc --static -g -o tracee tracee1.c
./tracer -p tracee

echo -e "\n** tracee2.c **\n"
gcc --static -g -o tracee tracee2.c
./tracer -p tracee