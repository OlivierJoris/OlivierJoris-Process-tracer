#include "file_sys_calls.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main(void){

    FileSysCalls* fsc = load_file();
    assert(fsc != NULL);
    assert(sys_calls_file_nb_elements(fsc) == 385);

    size_t nb_elements = sys_calls_file_nb_elements(fsc);

    for(size_t i = 0; i < nb_elements; ++i){
        printf("%lu: %s\n", (unsigned long)i, get_sys_call_name(fsc, (unsigned int)i));
    }

    sys_calls_file_free(fsc);
    fsc = NULL;

    return 0;
}
