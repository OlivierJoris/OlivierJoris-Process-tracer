/*
 * Module implementing the profiler interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "profiler.h"
#include "functions_addresses.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>

/*
 * Number of spaces representing a level of depth
 * inside the tree.
 */
#define NB_BLANKS 4

struct Profiler_t {
    char* tracee;
    pid_t childPID;
};

/*
 * Initialize the profiler.
 * 
 * @param tracee: path to the executable of the tracee.
 * 
 * @return Empty profiling data.
 */
static Profiler* init_profiler(char* tracee);

/*
 * Traces the function calls for the given profiler.
 * 
 * @param profiler: the profiler for which we want to
 * trace the function calls.
 */
static void trace_function_calls(Profiler* profiler);

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
        // Allows parent to trace the child process.
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        char* argv[] = {NULL};
        char* env[] = {NULL};
        close(1); // Doesn't show tracee output
        execve(profiler->tracee, argv, env);
    }else{
        profiler->childPID = childPID;
        trace_function_calls(profiler);
        printf("\nTracee finished\n");
    }

    return profiler;
}

static void trace_function_calls(Profiler* profiler){
    if(!profiler)
        return;
    FunctionsAddresses* fa = functions_addresses_load(profiler->tracee);
    if(!fa)
        return;

    unsigned long mainADDR = function_addresses_get_addr(fa, "main");
    if(mainADDR == 0){
        functions_addresses_clean(fa);
        return;
    }
    printf("Address of main = %lx\n", mainADDR);

    int status;
    struct user_regs_struct userRegs;
    bool nextIsCallee = false;
    bool running = true;
    unsigned long depth = 0, nbCalls = 0, nbRets = 0;
    // Used to get opcode on 1 byte
    const unsigned long PREFIX = 255;
    // Used to get opcode on 2 bytes - 255+2^12+2^13+2^14+2^15
    const unsigned long PREFIX2 = 61695;

    while(running){
        wait(&status);
        if(WIFEXITED(status))
            break;

        // Get registers
        ptrace(PTRACE_GETREGS, profiler->childPID, NULL, &userRegs);
            
        // Get instruction - EIP = 32 bits instruction register
        unsigned long instr = ptrace(PTRACE_PEEKTEXT, profiler->childPID, userRegs.eip, NULL);
        printf("%lx | ", userRegs.eip);
        if(nextIsCallee){
            nbCalls++;
            char* symbol = functions_addresses_get_symbol(fa, userRegs.eip);
            for(unsigned long i = 0; i < NB_BLANKS * depth; i++){
                //printf(" ");
            }
            //if(!symbol)
                //printf("Error fecth for %lx\n", userRegs.eip);
            //else
                //printf("%s\n", symbol);
            nextIsCallee = false;
            depth+=1;
        }

        // Opcode (on 1 byte) is the last 2 hex digits because ptrace uses big-endian
        unsigned long opcode = instr & PREFIX;
        // Opcode (on 2 bytes) is the last 4 hex digits because ptrace uses big-endian
        unsigned long opcode2 = instr & PREFIX2;

        printf("%lx | %lx\n", opcode, opcode2);
        //printf("Regiser eip content = %lx | ", userRegs.eip);
        //printf("instr = %lx | ", instr);
        //printf("opcode = %lx\n", opcode);

        // Opcodes for CALL
        if(opcode == 0xe8 || opcode == 0x9a || opcode == 0xcc || opcode == 0xcd || opcode == 0xf1
            || opcode == 0xce || opcode2 == 0x20ff || opcode2 == 0x30ff || opcode2 == 0x340f)
            nextIsCallee = true;

        // Opcodes for RET
        if(opcode == 0xc2 || opcode == 0xc3 || opcode == 0xca || opcode == 0xcb || opcode == 0xcf
            || opcode == 0XC9 || opcode2 == 0x350f){
            nbRets++;
            if(depth > 0)
                depth-=1;
            for(unsigned long int i = 0; i < NB_BLANKS * depth; i++){
                //printf(" ");
            }
            //printf("ret\n");
        }

        // Next instruction
        ptrace(PTRACE_SINGLESTEP, profiler->childPID, 0, 0);
    }

    printf("\n\n** Nb calls = %lu | nb rets = %lu **\n", nbCalls, nbRets);

    functions_addresses_clean(fa);

    return;
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
    printf("\n** PROFILER's DATA **\n");
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
