#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <sys/wait.h>

static int run_child(char *tracee);

static int run_this(pid_t child);

static int get_syscall(pid_t child);

int trace_syscalls(char *tracee)
{
    pid_t childPID = fork();

    if(childPID < 0)
        fprintf(stderr, "Failed forking process !\n");

    else 
    {
        if(childPID == 0)
            return run_child(tracee);

        else 
            return run_this(childPID);
    }
}

static int run_child(char *tracee)
{
    char *childArg[2];
    
    childArg[0] = tracee;

    childArg[1] = NULL;

    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    kill(getpid(), SIGSTOP);

    return execvp(childArg[0], childArg);
}

static int run_this(pid_t childPID)
{
    int status, syscallNumber;

    waitpid(childPID, &status, 0);
    ptrace(PTRACE_SETOPTIONS, childPID, NULL, PTRACE_O_TRACESYSGOOD);

    while(true) 
    {
        if(get_syscall(childPID) != 0) 
            break;

        syscallNumber = ptrace(PTRACE_PEEKUSER, childPID, sizeof(long) * ORIG_RAX);
        printf("syscall: %d\n", syscallNumber);
    }

    return 0;
}

static int get_syscall(pid_t childPID)
{
    int status;

    while(true) 
    {
        ptrace(PTRACE_SYSCALL, childPID, NULL, NULL);
        waitpid(childPID, &status, 0);
        
        if(WIFSTOPPED(status) && (WSTOPSIG(status) & 0x80))
            return 0;

        if(WIFEXITED(status))
            return 1;
    }
}
