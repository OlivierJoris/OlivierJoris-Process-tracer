#include "util.h"
#include "profiler.h"
#include "functions_addresses.h"

#include <stdio.h>

int main(int argc, char* argv[]){

    LaunchMode lm = load_arguments(argc, argv);

    switch (lm){
        case profiler:
            printf("Detected profiler mode\n");
            Profiler* profiler = run_profiler(argv[2]);
            profiler_display_data(profiler);
            profiler_clean(profiler);

            break;
        case syscall:
            printf("Detected syscall mode\n");
            /* TO BE FILLED */
            break;
        case error:
            fprintf(stderr, "Error while parsing arguments!\n");
            return -1;
    
        default:
            fprintf(stderr, "Invalid arguments!\n");
            return -1;
    }

    return 0;
}
