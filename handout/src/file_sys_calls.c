/*
 * Module implementing the file_sys_calls interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "file_sys_calls.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Represents the name of the file containing the mapping.
#define FILE_NAME "syscall.txt"

// Represents the size of the buffer for reading.
#define BUFFER_SIZE 64

struct FileSysCalls_t{
    char** mapping;
    size_t nb_elements;
};

FileSysCalls* load_file(void){

    FileSysCalls* fsc = malloc(sizeof(FileSysCalls));
    if(!fsc){
        fprintf(stderr, "Unable to allocate memory for loading the "
                        "mapping!\n");
        return NULL;
    }

    // First, need to know the number of elements
    FILE* f = fopen(FILE_NAME, "r");
    if(!f){
        fprintf(stderr, "Unable to open the file containing the mapping!\n");
        sys_calls_file_free(fsc);
        return NULL;
    }

    unsigned int curr_id, max_id = 0;
    char buffer[BUFFER_SIZE];
    while(fscanf(f, "%u %s\n", &curr_id, buffer) != -1){
        if(curr_id > max_id)
            max_id = curr_id;
    }
    fclose(f);
    f = NULL;

    // +1 because starts at 0
    fsc->nb_elements = (size_t)(max_id + 1);
    fsc->mapping = malloc(sizeof(char*) * fsc->nb_elements);
    if(!fsc->mapping){
        fprintf(stderr, "Unable to allocate memory for loading the "
                        "mapping!\n");
        sys_calls_file_free(fsc);
        return NULL;
    }

    // Needs to load the data
    f = fopen(FILE_NAME, "r");
    while(fscanf(f, "%u %s\n", &curr_id, buffer) != -1){
        fsc->mapping[curr_id] = malloc(sizeof(char) * BUFFER_SIZE);
        if(!fsc->mapping[curr_id]){
            fprintf(stderr, "Unable to allocate memory for loading the "
                            "mapping!\n");
            sys_calls_file_free(fsc);
            return NULL;
        }
        strcpy(fsc->mapping[curr_id], buffer);
    }

    fclose(f);
    f = NULL;

    return fsc;
}

void sys_calls_file_free(FileSysCalls* fsc){
    if(!fsc)
        return;
    if(!fsc->mapping){
        free(fsc);
        fsc = NULL;
        return;
    }
        
    for(size_t i = 0; i < fsc->nb_elements; ++i){
        if(fsc->mapping[i])
            free(fsc->mapping[i]);
    }

    free(fsc->mapping);
    free(fsc);
    fsc = NULL;

    return;
}

char* get_sys_call_name(FileSysCalls* fsc, unsigned int id){
    if(!fsc || !fsc->mapping || id >= fsc->nb_elements)
        return NULL;
    else
        return fsc->mapping[id];
}
