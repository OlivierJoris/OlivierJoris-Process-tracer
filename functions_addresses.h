/*
 * Interface to get the mapping between functions' symbols and
 * addresses.
 * 
 * @warning Requires nm & objdump to be available.
 * 
 * @author Maxime Goffart (180521) & Olivier Joris (182113)
 */

#ifndef __FUNCTIONS_ADDRESSES__
#define __FUNCTIONS_ADDRESSES__

/* 
 * Represents the mapping between functions' addresses and
 * symbols.
 */
typedef struct FunctionsAddresses_t FunctionsAddresses;

/*
 * Loads mapping between functions' addresses and symbols.
 * 
 * @warning Needs to call functions_addresses_clean() after usage.
 * 
 * @param exec Path to the executable for which we want the
 * mapping.
 * 
 * @return Pointer to the mapping.
 */
FunctionsAddresses* functions_addresses_load(char* exec);

/*
 * Cleans the mapping.
 * 
 * @param fa Pointer to the mapping.
 */
void functions_addresses_clean(FunctionsAddresses* fa);

/*
 * Gets the symbol associated to the given address.
 * 
 * @param fa Pointer to the mapping.
 * @param addr Address for which we want the associated symbol.
 * 
 * @return Symbol associated to the given address.
 */
char* functions_addresses_get_symbol(FunctionsAddresses* fa, 
                                     unsigned long addr);

#endif
