/*
 * Interface containing various utility functions.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#ifndef __UTIL__
#define __UTIL__

/* 
 * Represents the launch mode of the program.
 */
typedef enum LaunchMode_t
{
    profiler,
    syscall,
    error
}LaunchMode;

/*
 * Loads program's arguments.
 * 
 * @param argc: number of arguments.
 * @param argv: pointer to a string array containing program's arguments.
 * 
 * @return profiler If profiler mode.
 *         syscall  If syscall mode.
 *         error    Else.
 */
LaunchMode load_arguments(int argc, char **argv);

#endif
