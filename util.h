#ifndef __UTIL__
#define __UTIL__

/*
 * Load the program argument.
 * 
 * @param argc: number of arguments
 * @param argv: pointer to a string array containing the program arguments.
 * 
 * @return EXIT_SUCCESS If arguments are correctly loadead.
 *         EXIT_FAILURE Else.
 */

int load_arguments(int argc, char **argv);

#endif
