/*
 * Module implementing the tracer.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#include "load_param.h"
#include "profiler.h"
#include "functions_addresses.h"
#include "syscall.h"

#include <stdio.h>

int main(int argc, char* argv[]){

    Mode lm = load_arguments(argc, argv);

    switch (lm){
        case profiler:{
            Profiler* p = run_profiler(argv[2]);
            if(!p){
                fprintf(stderr, "Error while running profiler!\n");
                return -1;
            }
            profiler_display_data(p);
            profiler_clean(p);
            break;
        }

        case syscall:{
            FileSysCalls* fsc = load_file();
            if(!fsc){
                fprintf(stderr, "Syscall: unable to load the mapping!\n");
                return -1;
            }
            if(trace_syscalls(argv[2], fsc) == EXIT_SUCCESS){
                sys_calls_file_free(fsc);
                break;
            }else{
                sys_calls_file_free(fsc);
                return -1;
            }
        }

        case error:
            fprintf(stderr, "Error while parsing arguments!\n");
            return -1;
    
        default:
            fprintf(stderr, "Invalid arguments!\n");
            return -1;
    }

    return 0;
}
