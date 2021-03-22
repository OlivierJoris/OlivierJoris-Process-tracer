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

/*
 * Represents a call to a function.
 */
typedef struct Func_call_t Func_call;
struct Func_call_t{
    Func_call* prev;            // Previous called function (parent)
    unsigned long addr;         // Address of beginning of function call
    char* name;                 // Name of the function
    unsigned int nbInstr;       // Number of local instructions
    unsigned int nbInstrChild;  // Number of instructions (including children)
    unsigned int nbRecCalls;    // Number of rec calls (0 -> no recursivity)
    Func_call* children;        // Children function
    Func_call* next;            // Next function
    unsigned int depth;         // Depth inside call tree
};

struct Profiler_t{
    char* tracee;               // Tracee's name
    pid_t childPID;             // PID of tracee
    Func_call* entryPoint;      // Entry point of the call tree
};

/*
 * Initializes the profiler.
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

/*
 * Creates a node representing a function call.
 * 
 * @return New node.
 */
static Func_call* func_call_create_node(void);

/*
 * Sets the fields of the new node.
 * 
 * @param new Node to be set.
 * @param prev Link to parent of new node.
 * @param newDepth Depth of the new node.
 * @param Symbol of function of new node.
 */
static void func_call_set(Func_call* new, Func_call* prev, unsigned int newDepth, char* symbol);

/*
 * Frees the memory aasociated to a Func_call.
 * 
 * @param fc Func_call to free.
 */
static void func_call_free(Func_call* fc);

/*
 * Prints the function calls as in the brief.
 * 
 * @param fc Start of the function calls.
 */
static void func_call_print(Func_call* fc);

/*
 * Prints a function as in the brief (function's name and
 * number of instructions including children).
 * 
 * @param fc Function to print.
 */
static void func_call_print_unique(Func_call* fc);

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

    int status;
    struct user_regs_struct userRegs;
    bool nextIsCallee = false;
    bool running = true;
    unsigned long depth = 0, prevLocalDepth = 0, nbCalls = 0, nbRets = 0;
    // Used to get opcode on 1 byte
    const unsigned long PREFIX = 255;
    // Used to get opcode on 2 bytes - 2^16-1
    const unsigned long PREFIX2 = 65535;
    // Used to get opcode on 2 bytes excluding 3d hex digit.
    const unsigned long PREFIX3 = 61695;

    bool nextIsDeref = false;
    unsigned long comingAddr;

    profiler->entryPoint = func_call_create_node();
    if(!profiler->entryPoint)
        return;
    
    Func_call* currNode = profiler->entryPoint;
    unsigned int prevDepth;
    char* prevFuncName = calloc(256, sizeof(char));
    if(!prevFuncName)
        return;

    while(running){
        wait(&status);
        if(WIFEXITED(status))
            break;

        // Get registers
        ptrace(PTRACE_GETREGS, profiler->childPID, NULL, &userRegs);
            
        // Get instruction - EIP = 32 bits instruction register
        unsigned long instr = ptrace(PTRACE_PEEKTEXT, profiler->childPID, userRegs.eip, NULL);

        if(nextIsCallee){
            nbCalls++;
            char* symbol = functions_addresses_get_symbol(fa, userRegs.eip);
            char* symbolDeref;
            for(unsigned long i = 0; i < NB_BLANKS * depth; i++)
                printf(" ");
            if(!symbol){
                if(nextIsDeref){
                    symbolDeref = function_address_get_symbol_deref(profiler->tracee, comingAddr);
                    printf("%s | %lu\n", symbolDeref, depth);
                    nextIsDeref = 0;
                }
            }
            else
                printf("%s | %lu\n", symbol, depth);

            // Updates number of instructions recursively
            Func_call* tmp_prev = currNode;
            prevDepth = currNode->depth;
            currNode->nbInstrChild = currNode->nbInstr;
            while(tmp_prev != NULL){
                while(tmp_prev != NULL && tmp_prev->depth != prevDepth - 1)
                    tmp_prev = tmp_prev->prev;
                if(tmp_prev != NULL){
                    tmp_prev->nbInstrChild+=currNode->nbInstr;
                    prevDepth = tmp_prev->depth;
                }
            }

            // Update structure of the tree
            Func_call* tmp_next;
            // Recursive
            if(prevFuncName && depth == prevLocalDepth + 1 && symbol && !strcmp(symbol, prevFuncName)){
                currNode->nbRecCalls+=1;
                tmp_next = currNode;
            }else{
                // Need to update next field.
                if(depth == currNode->depth || depth == currNode->depth - 1){
                    if(depth == currNode->depth - 1){
                        // Go back until we reach same depth
                        while(currNode->depth > depth)
                            currNode = currNode->prev;
                    }

                    currNode->next = func_call_create_node();
                    if(!currNode->next)
                        return;
                    tmp_next = currNode->next;
                }else{
                    // Need to update children field.
                    currNode->children = func_call_create_node();
                    if(!currNode->children)
                        return;
                    tmp_next = currNode->children;
                }
                if(symbol != NULL){
                    func_call_set(tmp_next, currNode, depth, symbol);
                    strcpy(prevFuncName, symbol);
                }else{
                    func_call_set(tmp_next, currNode, depth, symbolDeref);
                    strcpy(prevFuncName, symbolDeref);
                    free(symbolDeref);
                    symbolDeref = NULL;
                }
            }
            prevLocalDepth = depth;
            currNode = tmp_next;

            nextIsCallee = false;
            depth+=1;
        }

        // Opcode (on 1 byte) is the last 2 hex digits because ptrace uses big-endian
        unsigned long opcode = instr & PREFIX;
        // Opcode (on 2 bytes) is the last 4 hex digits because ptrace uses big-endian
        unsigned long opcode2 = instr & PREFIX2;
        // Opcode (on 2 bytes) excluding 3d hex digit.
        unsigned long opcode3 = instr & PREFIX3;

        // Opcodes for CALL
        if(opcode == 0xe8 || opcode == 0x9a || opcode3 == 0xd0ff || opcode3 == 0x10ff || opcode3 == 0x50ff || opcode3 == 0x90ff){
            nextIsCallee = true;
            if(opcode2 == 0x15ff){
                nextIsDeref = true;
                comingAddr = userRegs.eip;
            }
        }

        // Opcodes for RET
        if(opcode == 0xc2 || opcode == 0xc3 || opcode == 0xca || opcode == 0xcb || opcode2 == 0xc3f3 || opcode2 == 0xc3f2){
            nbRets++;
            if(depth > 0)
                depth-=1;
        }

        currNode->nbInstr++; // update number of instr

        // Next instruction
        ptrace(PTRACE_SINGLESTEP, profiler->childPID, 0, 0);
    }

    // Last recursive update to number of instructions
    Func_call* tmp_prev = currNode;
    prevDepth = currNode->depth;
    currNode->nbInstrChild = currNode->nbInstr;
    while(tmp_prev != NULL){
        while(tmp_prev != NULL && tmp_prev->depth != prevDepth - 1)
            tmp_prev = tmp_prev->prev;
        if(tmp_prev != NULL){
            tmp_prev->nbInstrChild+=currNode->nbInstr;
            prevDepth = tmp_prev->depth;
        }
    }

    free(prevFuncName);

    printf("\n\n** Nb calls = %lu | nb rets = %lu **\n", nbCalls, nbRets);

    functions_addresses_clean(fa);

    return;
}

void profiler_clean(Profiler* profiler){
    if(!profiler)
        return;
    if(profiler->tracee)
        free(profiler->tracee);
    if(profiler->entryPoint)
        func_call_free(profiler->entryPoint);
    free(profiler);
    return;
}

void profiler_display_data(Profiler* profiler){
    // For submit, should respect given format.
    printf("\n** PROFILER's DATA **\n");
    printf("Tracee name = %s\n", profiler->tracee);
    if(profiler->entryPoint && profiler->entryPoint->next)
        func_call_print(profiler->entryPoint->next);
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

    profiler->entryPoint = NULL;

    return profiler;
}

Func_call* func_call_create_node(void){
    Func_call* node = malloc(sizeof(Func_call));
    if(!node)
        return NULL;

    node->prev = NULL;
    node->name = NULL;
    node->nbInstr = 0;
    node->nbInstrChild = 0;
    node->nbRecCalls = 0;
    node->children = NULL;
    node->next = NULL;
    node->depth = 0;
    
    return node;
}

void func_call_set(Func_call* new, Func_call* prev, unsigned int newDepth, char* symbol){
    if(!new)
        return;

    new->prev = prev;
    new->depth = newDepth;
    if(symbol){
        size_t length = strlen(symbol) + 1;
        new->name = malloc(sizeof(char) * length);
        if(!new->name)
            return;
        strcpy(new->name, symbol);
    }

    return;
}

void func_call_print(Func_call* fc){
    if(!fc)
        return;
    
    Func_call* tmp = fc;
    while(tmp != NULL){
        func_call_print_unique(tmp);
        if(tmp->children)
            func_call_print(tmp->children);
        tmp = tmp->next;
    }
}

void func_call_print_unique(Func_call* fc){
    if(!fc)
        printf("func_call_print_unique: null ptr!");
    for(unsigned int i = 0; i < NB_BLANKS * fc->depth; ++i)
        printf(" ");
    if(fc->name)
        printf("%s", fc->name);
    else
        printf("func_call_print_unique: unable to get name!\n");
    if(fc->nbRecCalls != 0)
        printf(" [rec call: %u]", fc->nbRecCalls);
    printf(": %u | %u | depth = %u\n", fc->nbInstr, fc->nbInstrChild, fc->depth);

    return;
}

void func_call_free(Func_call* fc){
    if(!fc)
        return;

    Func_call* tmp = fc;
    while(tmp != NULL){
        if(tmp->name){
            free(tmp->name);
            tmp->name = NULL;
        }
        if(tmp->children){
            func_call_free(tmp->children);
            tmp->children = NULL;
        }
        Func_call* tmp2 = tmp->next;
        if(tmp){
            free(tmp);
            tmp = NULL;
        }
        tmp = tmp2;
    }
}