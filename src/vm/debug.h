
//
//  Debug
//

#ifndef DEBUG_H
#define DEBUG_H

#include "vm.h"

// Pretty prints `instruction` to the standard output. `index` specifies the
// index of the instruction in the bytecode, used to calculate the destination
// for a jump instruction.
void debug_instruction(uint32_t index, uint64_t instruction);

// Pretty prints `fn`'s bytecode to the standard output.
void debug_bytecode(Function *fn);

#endif
