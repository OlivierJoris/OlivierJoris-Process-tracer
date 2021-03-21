/*
 * Interface to get mapping between function symbols and
 * addresses. Requires nm command to be available.
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
 * Need to call functions_addresses_clean() after usage.
 * 
 * @param exec: path to the executable for which we want the
 * mapping.
 * 
 * @return Pointer to the mapping.
 */
FunctionsAddresses* functions_addresses_load(char* exec);

/*
 * Cleans the mapping.
 * 
 * @param fa: pointer to the mapping.
 */
void functions_addresses_clean(FunctionsAddresses* fa);

/*
 * Gets the symbol associated to the given address.
 * 
 * @param fa: pointer to the mapping.
 * @param addr: address for which we want the associated symbol.
 * 
 * @return Symbol associated to the given address.
 */
char* functions_addresses_get_symbol(FunctionsAddresses* fa, unsigned long addr);

/*
 * Gets the symbol associated to the function called by assembly: call *0x80....
 * 
 * @param tracee Path to the executable of the tracee.
 * @param addr Address of instruction when the call occured.
 * 
 * @return Name of called function.
 */
char* function_address_get_symbol_deref(char* tracee, unsigned long addr);

/*
 * Gets the address associated to the given symbol.
 * 
 * @param fa: pointer to the mapping.
 * @param symbol: symbol for which we want the associated address.
 * 
 * @return Address associated to the given symbol.
 */
unsigned long function_addresses_get_addr(FunctionsAddresses* fa, char* symbol);

#endif
