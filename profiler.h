/*
 * Interface for the profiler mode of the tracer.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#ifndef __PROFILER__
#define __PROFILER__

/*
 * Represents the data of the profiler.
 */
typedef struct Profiler_t Profiler;

/*
 * Runs the profiler on the given tracee.
 * 
 * @warning: Need to call profiler_clean() after usage.
 * 
 * @param tracee Path to the executable of the tracee.
 * 
 * @return Data of the profiling of the given tracee.
 */
Profiler* run_profiler(char* tracee);

/*
 * Cleans the profiling data.
 * 
 * @param profiler Data of the profiling of the tracee.
 */
void profiler_clean(Profiler* profiler);

/*
 * Displays the data of the profiling accroding to the given 
 * pattern.
 * 
 * @warning: must call run_profiler(char*) before.
 * 
 * @param profiler Data of the profiling of the tracee.
 */
void profiler_display_data(Profiler* profiler);

#endif
