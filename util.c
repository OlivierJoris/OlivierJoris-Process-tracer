/*
 * Module implementing the utility interface.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LaunchMode load_arguments(int argc, char **argv)
{
    if(argc != 3)
    {
        fprintf(stderr, "Please launch this program with ./tracer <mode> <prog>\n");
        return error;
    }

    if(!strcmp(argv[1], "-p"))
    {
        printf("Program launched in profiler mode on the %s tracee.\n", argv[2]);
        return profiler;
    }

    else
    {
        if(!strcmp(argv[1], "-s"))
        {
            printf("Program launched in syscall mode on the %s tracee.\n", argv[2]);
            return syscall;
        }
        
        else 
        {
            fprintf(stderr, "Please choose among the proposed modes (profiler or syscall mode).\n");
            return error;
        }
    }
}
