/*
 * Module implementing the load_param interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "load_param.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Mode load_arguments(int argc, char **argv){
    if(argc != 3){
        fprintf(stderr, "Please launch this program with ./tracer "
                        "<mode> <prog>\n");
        return error;
    }

    if(!strcmp(argv[1], "-p"))
        return profiler;
        
    else{
        if(!strcmp(argv[1], "-s"))
            return syscall;
        else 
            return error;
    }
}
