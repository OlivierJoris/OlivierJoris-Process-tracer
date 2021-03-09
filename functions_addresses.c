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
 *the mappings.
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
static char* generate_command(char* exec);

FunctionsAddresses* functions_addresses_load(char* exec){
    if(!exec){
        fprintf(stderr, "Unable to retreive mapping!\n");
        return NULL;
    }

    FunctionsAddresses* fa = malloc(sizeof(FunctionsAddresses));
    if(!fa)
        return NULL;
    fa->first = malloc(sizeof(Mapping));
    if(!fa->first){
        functions_addresses_clean(fa);
        return NULL;
    }

    char* command = generate_command(exec);
    system(command);
    free(command);

    FILE* f = fopen("nm.txt", "r");
    if(!f){
        functions_addresses_clean(fa);
        return NULL;
    }

    unsigned long address;
    char buffer[128];
    Mapping* current = fa->first;
    while(fscanf(f, "%lx %s\n", &address, buffer) != -1){
        current->addr = address;
        current->symbol = malloc(sizeof(char) * 128);
        if(!current->symbol){
            functions_addresses_clean(fa);
            fclose(f);
            return NULL;
        }
        strcpy(current->symbol, buffer);
        current->next = malloc(sizeof(Mapping));
        if(!current->next){
            functions_addresses_clean(fa);
            fclose(f);
            return NULL;
        }
        current = current->next;
        current->symbol = NULL;
        current->next = NULL;
    }

    fclose(f);

    return fa;
}

static char* generate_command(char* exec){
    if(!exec)
        return NULL;

    char* buffer = calloc(256, sizeof(char));
    if(!buffer)
        return NULL;

    sprintf(buffer, "nm --numeric-sort %s | grep -oE \"[0-9a-z]{8}[ ]{1}[T]{1}[ ]{1}[A-Za-z_.]*\" | awk '{print $1\" \"$3}' > nm.txt", exec);

    printf("Command = %s\n", buffer);

    return buffer;
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

char* functions_addresses_get(FunctionsAddresses* fa, unsigned long addr){
    if(!fa | !fa->first)
        return NULL;

    Mapping* mapping = fa->first;
    while(mapping != NULL){
        if(mapping->addr == addr)
            return mapping->symbol;
        mapping = mapping->next;
    }

    return NULL;
}
