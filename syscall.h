/*
 * Interface to trace the system calls of a process.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#ifndef __SYSCALL__
#define __SYSCALL__

#include "file_sys_calls.h"

/*
 * Trace the syscalls of the given tracee.
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
