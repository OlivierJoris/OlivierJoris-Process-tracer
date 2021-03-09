#ifndef __SYSCALL__
#define __SYSCALL__

#include "file_sys_calls.h"

/*
 * Trace the syscalls on the given tracee.
 * 
 * @param tracee: path to the executable of the tracee.
 * @param fsc: pointer to FileSysCalls containing the mapping
 * between system call ids and names.
 * 
 * @return EXIT_SUCCESS If no error.
 *         EXIT_FAILURE Else.
 */
int trace_syscalls(char *tracee, FileSysCalls *fsc);

#endif
