/*
 * Module implementing the functions_addresses interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "functions_addresses.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char first_call = 1;

/*
 * Represents a node inside the linked list containing
 * the mapping.
 */
typedef struct Mapping_t Mapping;
struct Mapping_t{
    unsigned long addr;
    char* symbol;
    Mapping* next;
};

struct FunctionsAddresses_t {
    Mapping* first;
};

/*
 * Generate the command (to retreive the mapping) to be
 * executed based on the name of the executable.
 * 
 * @param exec: path to the executable for which we want the
 * mapping.
 * 
 * @return The command to be executed.
 */
static char* generate_command_nm(char* exec);

/*
 * Generate the command (to retreive the mapping) between
 * functions' addresses and symbols.
 * 
 * @param exec: path to the executable for which we want the mapping.
 * 
 * @return The command to be executed.
 */
static char* generate_command_objdump(char* exec);

/*
 * Creates a new cell of the list of mappings.
 * 
 * @param address Address of the function.
 * @param symbolLength Length of function's symbol.
 * 
 * @return A new cell.
 */
static Mapping* create_new_cell(unsigned long address, unsigned long symbolLength);

FunctionsAddresses* functions_addresses_load(char* exec){
    if(!exec){
        fprintf(stderr, "Unable to retreive mapping!\n");
        return NULL;
    }

    const int BUFFER_SIZE = 128;

    FunctionsAddresses* fa = malloc(sizeof(FunctionsAddresses));
    if(!fa)
        return NULL;
    fa->first = create_new_cell(0, BUFFER_SIZE);
    if(!fa->first){
        functions_addresses_clean(fa);
        return NULL;
    }

    char* command = generate_command_nm(exec);
    if(!command){
        functions_addresses_clean(fa);
        return NULL;
    }
    system(command);
    free(command);

    FILE* f = fopen("nm.txt", "r");
    if(!f){
        functions_addresses_clean(fa);
        return NULL;
    }

    unsigned long address;
    char buffer[BUFFER_SIZE];
    Mapping* current = fa->first;
    while(fscanf(f, "%lx %s\n", &address, buffer) != -1){
        current->addr = address;
        strcpy(current->symbol, buffer);

        current->next = create_new_cell(0, BUFFER_SIZE);
        if(!current->next){
            functions_addresses_clean(fa);
            fclose(f);
            return NULL;
        }
        current = current->next;
        current->next = NULL;
    }

    fclose(f);

    current = fa->first;
    Mapping* prev = NULL;
    char* command2 = generate_command_objdump(exec);
    if(!command2)
        return fa;

    system(command2);
    free(command2);

    FILE* f2 = fopen("func_names.txt", "r");
    if(!f2)
        return fa;

    while(fscanf(f2, "%lx %s\n", &address, buffer) != -1){
        current = fa->first;
        prev = NULL;
        while(current != NULL){
            if(current->addr == address) // Already have the mapping in memory
                break;
            // New cell at the beginning of the list
            if(prev == NULL && address < current->addr){
                Mapping* newCell = create_new_cell(address, BUFFER_SIZE);
                if(!newCell){
                    fclose(f2);
                    return fa;
                }
                strcpy(newCell->symbol, buffer);
                Mapping* tmp = fa->first;
                fa->first = newCell;
                newCell->next = tmp;
                break;
            }
            // New cell in the middle of the list
            if(prev != NULL && address > prev->addr && address < current->addr){
                Mapping* newCell = create_new_cell(address, BUFFER_SIZE);
                if(!newCell){
                    fclose(f2);
                    return fa;
                }
                strcpy(newCell->symbol, buffer);
                newCell->next = prev->next;
                prev->next = newCell;
                break;
            }

            prev = current;
            current = current->next;
        }
    }

    fclose(f2);

    return fa;
}

static Mapping* create_new_cell(unsigned long address, unsigned long symbolLength){
    Mapping* newCell = malloc(sizeof(Mapping));
    if(!newCell)
        return NULL;
    newCell->symbol = calloc(symbolLength, sizeof(char));
    if(!newCell->symbol){
        free(newCell);
        return NULL;
    }
    newCell->addr = address;
    return newCell;
}

static char* generate_command_nm(char* exec){
    if(!exec)
        return NULL;

    char* cmd = calloc(256, sizeof(char));
    if(!cmd)
        return NULL;

    sprintf(cmd, "nm -a --numeric-sort %s | grep -oE \"[0-9a-z]{8}[ ]{1}[a-zA-Z]{1}[ ]{1}[A-Za-z0-9_.]*\" | awk '{print $1\" \"$3}' > nm.txt", exec);

    return cmd;
}

static char* generate_command_objdump(char* exec){
    if(!exec)
        return NULL;
    
    char* cmd = calloc(512, sizeof(char));
    if(!cmd)
        return NULL;
    
    sprintf(cmd, "objdump -d %s | grep -oE \"[0-9a-zA-Z: ]*call[ ]*[0-9a-z ]*<.*>\" | awk '{print $2 \" \"$3}' | sort | uniq > func_names.txt", exec);

    return cmd;
}

void functions_addresses_clean(FunctionsAddresses* fa){
    if(!fa)
        return;

    Mapping* tmp = fa->first;
    Mapping* next;

    while(tmp != NULL){
        next = tmp->next;
        if(tmp->symbol)
            free(tmp->symbol);
        free(tmp);
        tmp = next;
    }

    free(fa);

    return;
}

char* functions_addresses_get_symbol(FunctionsAddresses* fa, unsigned long addr){
    if(!fa | !fa->first)
        return NULL;

    Mapping* mapping = fa->first;
    while(mapping != NULL){
        if(mapping->symbol != NULL && mapping->addr == addr)
            return mapping->symbol;
        mapping = mapping->next;
    }

    return NULL;
}

char* function_address_get_symbol_deref(char* tracee, unsigned long addr){
    static char buffer[512];

    if(first_call){
        sprintf(buffer, "objdump -sS %s > dump_s.txt", tracee);
        system(buffer);
        first_call = 0;
    }

    // Find intermediate address
    sprintf(buffer, "cat dump_s.txt | grep -oE \".*%lx.*ff 15.*call.*\" | awk '{print $NF}' | grep -oE \"8.*\" > going_addr.txt", addr);
    system(buffer);

    FILE* f = fopen("going_addr.txt", "r");
    if(!f)
        return NULL;
    unsigned long goingAddr = 0;
    fscanf(f, "%lx", &goingAddr);
    fclose(f);

    // Find dest address
    sprintf(buffer, "cat dump_s.txt | grep -oE \".*%lx [0-9a-z]{8}[ ]{1}\" | awk '{print $2}' > going_addr.txt", goingAddr);
    system(buffer);

    f = fopen("going_addr.txt", "r");
    if(!f)
        return NULL;
    char reachedAddr[10];
    fscanf(f, "%s", reachedAddr);
    fclose(f);

    char* tmp = malloc(sizeof(char) * (strlen(reachedAddr) + 1));
    if(!tmp)
        return NULL;
    unsigned int i = 0;
    for(; i < strlen(reachedAddr); ++i){
        if(i%2){
            tmp[i] = reachedAddr[strlen(reachedAddr) - i];
        }else{
            tmp[i] = reachedAddr[strlen(reachedAddr) - 2 - i];
        }
    }
    tmp[i] = '\0';

    // Find name of function
    sprintf(buffer, "cat dump_s.txt | grep -oE \".*%s.*<.*>\" | awk '{print $2}' | grep -oE \"[^<]{1}.*[^>]{1}\" > func_name.txt", tmp);
    system(buffer);
    free(tmp);

    f = fopen("func_name.txt", "r");
    if(!f)
        return NULL;
    
    char* funcName = calloc(256, sizeof(char));
    if(!funcName){
        fclose(f);
        return NULL;
    }

    fscanf(f, "%s", funcName);
    fclose(f);
    
    return funcName;
}

unsigned long function_addresses_get_addr(FunctionsAddresses* fa, char* symbol){
    if(!fa || !fa->first)
        return 0;

    unsigned long addr = 0;

    Mapping* mapping = fa->first;
    while(mapping!= NULL){
        if(!strcmp(symbol, mapping->symbol))
            return mapping->addr;
        mapping = mapping->next;
    }

    return addr;
}