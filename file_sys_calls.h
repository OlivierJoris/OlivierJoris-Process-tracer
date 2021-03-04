/*
 * Interface for loading and manipulating the
 * file containing the mapping between the system
 * calls' ids and names.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#ifndef __FILE_SYS_CALLS__
#define __FILE_SYS_CALLS__

#include <stdlib.h>

/* 
 * Represents the file containing mapping between
 * system calls' ids and names.
 */
typedef struct FileSysCalls_t FileSysCalls;

/*
 * Loads the file. Need to call sys_call_file_free() 
 * after usage.
 * 
 * @return Pointer to the structure containing the data.
 */
FileSysCalls* load_file(void);

/*
 * Free the memory associated the file.
 * 
 * @param fsc: Pointer to the data that need to be freed.
 */
void sys_calls_file_free(FileSysCalls* fsc);

/*
 * Returns the name associated with the system call 
 * which has the given ID.
 * 
 * @param fsc: Pointer to the data.
 * @param id: ID of the system call for which we want
 * to know the name.
 * 
 * @return The name of the system call with the given ID.
 */
char* get_sys_call_name(FileSysCalls* fsc, unsigned int id);

/* TESTING PURPOSE ONLY -- NEED TO BE REMOVED BEFORE SUBMIT */
size_t sys_calls_file_nb_elements(FileSysCalls* fsc);

#endif
