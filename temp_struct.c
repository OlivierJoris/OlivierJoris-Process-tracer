typedef struct Func_call_t{
    Func_call* prev;            // Previous called function (parent)
    char* name;                 // Name of the function
    unsigned int nbInstr;       // Number of local instructions
    unsigned int nbInstrChild;  // Number of instructions (including children)
    unsigned int nbRecCalls;    // Number of rec calls (0 -> no recursivity)
    Func_call* children;        // Children function
    Func_call* next;            // Next function
    unsigned int depth;         // Depth inside call tree
}Func_call;