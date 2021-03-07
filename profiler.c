/*
 * Module implementing the profiler interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "profiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>

struct Profiler_t {
    char* tracee;
};

/*
 * Initialize the profiler.
 * 
 * @param tracee: path to the executable of the tracee.
 * 
 * @return Empty profiling data.
 */
static Profiler* init_profiler(char* tracee);

Profiler* run_profiler(char* tracee){
    Profiler* profiler = init_profiler(tracee);
    if(!profiler){
        fprintf(stderr, "Unable to initialize profiler!\n");
        return NULL;
    }

    printf("Tracee path = %s\n", profiler->tracee);

    pid_t childPID = fork();
    if(childPID < 0){
        fprintf(stderr, "Failed forking process!\n");
        profiler_clean(profiler);
        return NULL;
    }else if(childPID == 0){
        // Allow parent to trace this process.
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        char* argv[] = {NULL};
        char* env[] = {NULL};
        execve(profiler->tracee, argv, env);
    }else{
        int status;
        struct user_regs_struct userRegs;
        while(true){
            wait(&status);
            if(WIFEXITED(status))
                break;
            // Get registers
            ptrace(PTRACE_GETREGS, childPID, NULL, &userRegs);
            // EIP = 32 bits instruction register
            printf("Regiser eip content = %lx | ", userRegs.eip);
            // Get opcode
            long opcode = ptrace(PTRACE_PEEKTEXT, childPID, userRegs.eip, NULL);
            printf("opcode = %lx\n", opcode);
            ptrace(PTRACE_SINGLESTEP, childPID, 0, 0);
        }
        printf("Tracee finished\n");
    }

    return profiler;
}

void profiler_clean(Profiler* profiler){
    if(!profiler)
        return;
    if(profiler->tracee)
        free(profiler->tracee);
    free(profiler);
    return;
}

void profiler_display_data(Profiler* profiler){
    // For submit, should respect given format.
    printf("** PROFILER's DATA **\n");
    printf("Tracee name = %s\n", profiler->tracee);
    return;
}

Profiler* init_profiler(char* tracee){
   if(!tracee){
        fprintf(stderr, "Error given tracee!\n");
        return NULL;
    }

    Profiler* profiler = malloc(sizeof(Profiler));
    if(!profiler)
        return NULL;
    size_t strLen = strlen(tracee) + 1;
    profiler->tracee = malloc(sizeof(char) * strLen);
    if(!profiler->tracee){
        profiler_clean(profiler);
        return NULL;
    }
    strcpy(profiler->tracee, tracee);

    return profiler;
}
