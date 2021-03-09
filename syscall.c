/*
 * Module implementing the syscall interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "syscall.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>

/*
 * Checks if a system call has occured in the tracee process.
 * 
 * @param tracee: pid of the process running the tracee.
 * 
 * @return true If a syscall has been detected in the tracee.
 *         false If the tracee exited.
 */
static bool is_syscall(pid_t tracee);

/*
 * Given a system call number, returns his name.
 * 
 * @param fsc: pointer to FileSysCalls containing the mapping
 * between system call ids and names.
 * @param syscallNumber: system call number id.
 * 
 * @return The name of the given system call number id.
 */
static inline char *from_syscall_number_to_name(FileSysCalls* fsc, 
                                                unsigned int syscallNumber);

int trace_syscalls(char *tracee, FileSysCalls *fsc)
{
    pid_t traceePID = fork();

    if(traceePID < 0)
    {
        fprintf(stderr, "Failed forking process !\n");
        return EXIT_FAILURE;
    }

    else 
    {
        if(traceePID == 0)
        {
            char* argv[] = {NULL};
            char* env[] = {NULL};

            ptrace(PTRACE_TRACEME, 0, NULL, NULL);
            close(1);
            return execve(tracee, argv, env);
        }

        else 
        {
            int status;
            unsigned int syscallNumber;

            waitpid(traceePID, &status, 0);
            ptrace(PTRACE_SETOPTIONS, traceePID, NULL, PTRACE_O_TRACESYSGOOD);

            while(true) 
            {
                if(!is_syscall(traceePID)) 
                    break;

                syscallNumber = ptrace(PTRACE_PEEKUSER, traceePID,
                                       sizeof(long) * ORIG_EAX);

                printf("syscall: %s\n", from_syscall_number_to_name(fsc,
                                                            syscallNumber));

                if(!is_syscall(traceePID)) 
                    break;
            }

            return EXIT_SUCCESS;
        }
    }
}

static bool is_syscall(pid_t traceePID)
{
    int status;

    while(true) 
    {
        ptrace(PTRACE_SYSCALL, traceePID, NULL, NULL);
        waitpid(traceePID, &status, 0);

        if(WIFEXITED(status))
            return false;
        
        // 0x80 = 0b1000000000000000 (check if high bit is set)
        if(WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80)) 
            return true;
    }
}

static inline char *from_syscall_number_to_name(FileSysCalls* fsc, 
                                                unsigned int syscallNumber)
{
    return get_sys_call_name(fsc, syscallNumber);
}
