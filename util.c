#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

/* 
 * Represents the launch mode of the program..
 */
typedef enum
{
    profiler,
    syscall
}LaunchMode;

int load_arguments(int argc, char **argv)
{
    if(argc != 3)
    {
        fprintf(stderr, "Please launch this program with ./tracer <mode> <prog>\n");
        return EXIT_FAILURE;
    }

    LaunchMode mode;

    if(!strcmp(argv[1], "-p"))
    {
        mode = profiler;
        printf("Program launched in profiler mode on the %s tracee\n.", argv[2]);
        return EXIT_SUCCESS;
    }
    else
    {
        if(!strcmp(argv[1], "-m"))
        {
            mode = syscall;
            printf("Program launched in syscall mode on the %s tracee\n.", argv[2]);
            return EXIT_SUCCESS;
        }
        else 
            fprintf(stderr, "Please choose among the proposed modes (profiler or syscall mode).\n");
            return EXIT_FAILURE;
    }
}
