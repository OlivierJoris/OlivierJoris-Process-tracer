/*
 * Module implementing the functions_addresses interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "functions_addresses.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Represents a node inside the linked list containing
 * the mapping.
 */
typedef struct Mapping_t Mapping;
struct Mapping_t{
    unsigned long addr; // Entry point of the function
    char* symbol;       // Symbol associated to the function
    Mapping* next;      // Next mapping in the list
};

struct FunctionsAddresses_t {
    Mapping* first;     // Pointer to the beginning of the list
};

/*
 * Generates the command (to retreive the mapping) to be
 * executed based on the name of the executable.
 * 
 * @param exec Path to the executable for which we want the
 * mapping.
 *
 * @warning Requires nm to be available.
 * 
 * @return The command to be executed.
 */
static char* generate_command_nm(char* exec);

/*
 * Generates the command (to retreive the mapping) between
 * functions' addresses and symbols.
 * 
 * @param exec Path to the executable for which we want the
 * mapping.
 *
 * @warning Requires objdump to be available.
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
static Mapping* create_new_cell(unsigned long address, 
                                unsigned long symbolLength);

FunctionsAddresses* functions_addresses_load(char* exec){
    if(!exec){
        fprintf(stderr, "Unable to retreive mapping!\n");
        return NULL;
    }

    static const int BUFFER_SIZE = 128; // Size of a string

    FunctionsAddresses* fa = malloc(sizeof(FunctionsAddresses));
    if(!fa){
        fprintf(stderr, "Unable to allocate memory to fetch the mapping!\n");
        return NULL;
    }

    fa->first = create_new_cell(0, BUFFER_SIZE);
    if(!fa->first){
        fprintf(stderr, "Unable to allocate memory to fetch the mapping!\n");
        functions_addresses_clean(fa);
        return NULL;
    }

    // Get functions' names based on nm.

    char* command = generate_command_nm(exec);
    if(!command){
        fprintf(stderr, "Unable to generate the command to fetch the "
                        "mapping!\n");
        functions_addresses_clean(fa);
        return NULL;
    }
    system(command);
    free(command);

    FILE* f = fopen("nm.txt", "r");
    if(!f){
        fprintf(stderr, "Unable to open file to fetch the mapping!\n");
        functions_addresses_clean(fa);
        return NULL;
    }

    unsigned long addressPrev = 0, address;
    char buffer[BUFFER_SIZE], typePrev = '\0', type = '\0';
    Mapping* current = fa->first;
    while(fscanf(f, "%lx %c %s\n", &address, &type, buffer) != -1){
        // Prefer symbol from text than weak symbol
        if(addressPrev == address &&
            (type == 'W' || type == 'w') &&
            (typePrev == 'T' || typePrev == 't'))
            continue;
        // Saves current line in the file
        current->addr = address;
        strcpy(current->symbol, buffer);

        // Builds new cell in the list
        current->next = create_new_cell(0, BUFFER_SIZE);
        if(!current->next){
            fprintf(stderr, "Unable to allocate memory to fetch the "
                            "mapping!\n");
            functions_addresses_clean(fa);
            fclose(f);
            return NULL;
        }
        current = current->next;
        current->next = NULL;
        addressPrev = address;
        typePrev = type;
    }

    fclose(f);

    // Get functions' names based on objdump.

    char* command2 = generate_command_objdump(exec);
    if(!command2){
        fprintf(stderr, "Unable to generate the command to fetch the "
                        "mapping!\n");
        functions_addresses_clean(fa);
        return NULL;
    }

    system(command2);
    free(command2);

    FILE* f2 = fopen("func_names.txt", "r");
    if(!f2){
        fprintf(stderr, "Unable to open file to fetch the mapping!\n");
        functions_addresses_clean(fa);
        return NULL;
    }

    current = fa->first;
    Mapping* prev = NULL;
    while(fscanf(f2, "%lx %s\n", &address, buffer) != -1){
        current = fa->first;
        prev = NULL;
        while(current){
            // Already have the mapping in memory
            if(current->addr == address)
                break;

            // New cell at the beginning of the list
            if(!prev && address < current->addr){
                Mapping* newCell = create_new_cell(address, BUFFER_SIZE);
                if(!newCell){
                    fprintf(stderr, "Unable to allocate memory to fetch the "
                                    "mapping!\n");
                    functions_addresses_clean(fa);
                    fclose(f2);
                    return NULL;
                }

                strcpy(newCell->symbol, buffer);
                Mapping* tmp = fa->first;
                fa->first = newCell;
                newCell->next = tmp;
                break;
            }

            // New cell in the middle of the list
            if(prev && address > prev->addr &&
               address < current->addr){
                Mapping* newCell = create_new_cell(address, BUFFER_SIZE);
                if(!newCell){
                    fprintf(stderr, "Unable to allocate memory to fetch the "
                                    "mapping!\n");
                    functions_addresses_clean(fa);
                    fclose(f2);
                    return fa;
                }
                // Remove <>
                if(buffer[0] == '<' && buffer[strlen(buffer)-1] == '>'){
                    for(size_t i = 0; i < strlen(buffer) - 1; ++i){
                        buffer[i] = buffer[i+1];
                    }
                    if(strlen(buffer) >= 2)
                        buffer[strlen(buffer)-2] = '\0';
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

static Mapping* create_new_cell(unsigned long address, 
                                unsigned long symbolLength){
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

    sprintf(cmd, "nm --numeric-sort %s | grep -oE "
                 "\"[0-9a-z]{8}[ ]{1}[TtVvWw]{1}[ ]{1}[A-Za-z0-9_.]*\" | awk "
                 "'{print $1\" \"$2\" \"$3}' | sort | uniq > nm.txt", exec);

    return cmd;
}

static char* generate_command_objdump(char* exec){
    if(!exec)
        return NULL;
    
    char* cmd = calloc(256, sizeof(char));
    if(!cmd)
        return NULL;
    
    sprintf(cmd, "objdump -d %s | grep -oE "
                 "\"[0-9a-zA-Z: ]*call[ ]*[0-9a-z ]*<.*>\" | awk "
                 "'{print $2 \" \"$3}' | sort | uniq > func_names.txt", exec);

    return cmd;
}

void functions_addresses_clean(FunctionsAddresses* fa){
    if(!fa)
        return;

    Mapping* tmp = fa->first;
    Mapping* next;

    while(tmp){
        next = tmp->next;
        if(tmp->symbol)
            free(tmp->symbol);
        free(tmp);
        tmp = next;
    }

    free(fa);

    return;
}

char* functions_addresses_get_symbol(FunctionsAddresses* fa, 
                                     unsigned long addr){
    if(!fa || !fa->first)
        return NULL;

    Mapping* mapping = fa->first;
    while(mapping){
        if(mapping->symbol && mapping->addr == addr)
            return mapping->symbol;
        mapping = mapping->next;
    }

    return NULL;
}
