
//
//  Debug
//

#ifndef DEBUG_H
#define DEBUG_H

#include "vm.h"

// Pretty prints `instruction` to the standard output.
void debug_print_instruction(int i, uint64_t instruction);

// Pretty prints all instructions in a function's bytecode to the standard
// output.
void debug_print_bytecode(Function *fn);

#endif
