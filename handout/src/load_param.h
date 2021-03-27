/*
 * Interface containing the function to load the arguments.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#ifndef __LOAD_PARAM__
#define __LOAD_PARAM__

/* 
 * Represents the mode of the tracer.
 */
typedef enum Mode_t
{
    profiler,
    syscall,
    error
}Mode;

/*
 * Loads program's arguments.
 * 
 * @param argc Number of arguments.
 * @param argv Pointer to a string array containing program's arguments.
 * 
 * @return profiler If profiler mode.
 *         syscall  If syscall mode.
 *         error    Else.
 */
Mode load_arguments(int argc, char **argv);

#endif
