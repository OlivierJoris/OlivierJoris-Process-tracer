#include "util.h"
#include "profiler.h"
#include "functions_addresses.h"
#include "syscall.h"

#include <stdio.h>

int main(int argc, char* argv[]){

    LaunchMode lm = load_arguments(argc, argv);

    switch (lm)
    {
        case profiler:
            printf("Detected profiler mode\n");
            Profiler* profiler = run_profiler(argv[2]);
            profiler_display_data(profiler);
            profiler_clean(profiler);

            FunctionsAddresses* fa = functions_addresses_load(argv[2]);
            char* symbol = functions_addresses_get(fa, 0x0804887c); //main of tracee
            printf("Symbol = %s\n", symbol);
            functions_addresses_clean(fa);
            break;
        case syscall:
            printf("Detected syscall mode\n");
            FileSysCalls *fsc = load_file();
            if(trace_syscalls(argv[2], fsc) == 0)
            {
                sys_calls_file_free(fsc);
                break;
            }
                
            else
            {
                sys_calls_file_free(fsc);
                return -1;
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
